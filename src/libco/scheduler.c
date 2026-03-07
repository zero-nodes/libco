#include "libco/scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10

static void empty() {};

scheduler_t* scheduler_create()  
{
    scheduler_t *scheduler;
    scheduler = malloc(sizeof(scheduler_t));

    if (!scheduler)
    {
        fprintf(stderr, "error malloc get scheduler");
        return NULL;
    }
    
    memset(scheduler, 0, sizeof(scheduler_t));

    scheduler->len_start_list = 0;
    scheduler->status = INIT_SCHEDULER;
    scheduler->start_list = NULL;
    scheduler->cap_start_list = 0;
    scheduler->current_index = 0;
    scheduler->epoll_fd = -1;
    
    scheduler->ctx = context_create(empty, 0);

    if (!scheduler->ctx) {
        fprintf(stderr, "error create scheduler context\n");
        free(scheduler);
        return NULL;
    }

    return scheduler;
}

void scheduler_free(scheduler_t *scheduler)
{
    if(scheduler)
    {
        if (scheduler->ctx)
        {
            context_free(scheduler->ctx);
            scheduler->ctx = NULL;
        }
        
        if (scheduler->start_list)
        {
            free(scheduler->start_list);
            scheduler->start_list = NULL;
        }
        
        if (scheduler->epoll_fd != -1) {
            close(scheduler->epoll_fd);
            scheduler->epoll_fd = -1;
        }
        
        free(scheduler);
    }
}
int scheduler_add_coroutine_to_start(scheduler_t *scheduler, coroutine_t *coro)
{
    if (!scheduler) 
        return -1;

    if (!coro)
        return -1;

    coro->scheduler = scheduler;
    coro->state = WAITTING_COROUTINE;

    if (scheduler->len_start_list >= scheduler->cap_start_list)
    {
        size_t new_cap = scheduler->cap_start_list ? scheduler->cap_start_list * 2 : 4;
        coroutine_t **new_list = realloc(scheduler->start_list, new_cap * sizeof(coroutine_t*));
        
        if (!new_list) {
            perror("realloc failed");
            exit(EXIT_FAILURE); 
        }

        scheduler->start_list = new_list;
        scheduler->cap_start_list = new_cap;
    }

    scheduler->start_list[scheduler->len_start_list] = coro;
    scheduler->len_start_list++;
    scheduler->count_not_ready_coroutine++;

    return 0;
}

int scheduler_start(scheduler_t *scheduler)
{

    scheduler->status = RUNNING_SCHEDULER;

    if (!scheduler)
    {
        fprintf(stderr, "error start scheduler bot init scheduler");
        return -1;
    }

    scheduler->epoll_fd = epoll_create1(0);
    if (scheduler->epoll_fd == -1) {
        fprintf(stderr, "error epoll_create1");
        return -1;
    }

    while(1)
    {

        for (;scheduler->current_index < scheduler->len_start_list; scheduler->current_index++)
        {
            context_switch(scheduler->ctx, scheduler->start_list[scheduler->current_index]->coroutine_ctx);
            if (scheduler->start_list[scheduler->current_index]->state == FINISHED_COROUTINE)
            {
                coroutine_t *coro = scheduler->start_list[scheduler->current_index];
                if (coro->wait_fd != -1) {
                    epoll_ctl(scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                }
                coroutine_free(coro);
                scheduler->count_not_ready_coroutine--;
            }        
        }

        if (scheduler->count_not_ready_coroutine <= 0)
        {
            break;
        }

        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(scheduler->epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < nfds; i++) 
        {
            coroutine_t *coro = (coroutine_t*)events[i].data.ptr;
            coro->event_buffer = events[i].events;
            
            if (coro->wait_fd != -1) {
                epoll_ctl(scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                coro->wait_fd = -1;
            }
            
            context_switch(scheduler->ctx, coro->coroutine_ctx);

            if (coro->state == FINISHED_COROUTINE)
            {
                coroutine_free(coro);
                scheduler->count_not_ready_coroutine--;
            }
        }    
    }
    
    return 0;
}

int scheduler_ctl_add(scheduler_t *scheduler, coroutine_t *coro, int fd, uint32_t event_type)
{
    struct epoll_event event;

    memset(&event, 0, sizeof(event));

    event.events = event_type | EPOLLET;
    event.data.ptr = coro;

    if (coro->wait_fd != -1) 
    {
        epoll_ctl(scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
    }

    coro->wait_fd = fd;

    return epoll_ctl(scheduler->epoll_fd, EPOLL_CTL_ADD, fd, &event);
}
