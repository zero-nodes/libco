#include "libco/scheduler.h"
#include "context/context.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>

#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

static void empty() {};

scheduler_t* scheduler_create(int thread_count)  
{
    scheduler_t *scheduler;
    scheduler = malloc(sizeof(scheduler_t));

    if (!scheduler)
    {
        fprintf(stderr, "error malloc get scheduler\n");
        return NULL;
    }
    
    memset(scheduler, 0, sizeof(scheduler_t));

    scheduler->queue = scheduler_queue_create();
    if (!scheduler->queue)
    {
        fprintf(stderr, "error create queue\n");
        return NULL;
    }

    scheduler->status = INIT_SCHEDULER;
    scheduler->epoll_fd = -1;
    scheduler->stop_flag = 0;
    
    scheduler->thread_count = thread_count;
    scheduler->threads = malloc(sizeof(pthread_t) * thread_count);
    if (!scheduler->threads)
    {
        fprintf(stderr, "error malloc get theads\n");
        scheduler_free(scheduler);
        return NULL;
    }

    atomic_init(&scheduler->count_not_ready_coroutine, 0);

    return scheduler;
}

void scheduler_free(scheduler_t *scheduler)
{
    if (!scheduler)
    {
        return;
    }
    if(scheduler->queue)
    {
        scheduler_queue_free(scheduler->queue);
    }
    if(scheduler->threads)
    {
        free(scheduler->threads);
    }
    if (scheduler->epoll_fd != -1) {
        close(scheduler->epoll_fd);
        scheduler->epoll_fd = -1;
    }
    if(scheduler)
    {
        free(scheduler);
    }
}

int scheduler_add_coroutine_to_start(scheduler_t *scheduler, coroutine_t *coro)
{
    if (!scheduler) 
        return -1;

    if (!coro)
        return -1;

    coro->state = WAITTING_COROUTINE;
    coro->scheduler = scheduler;

    scheduler_queue_push(scheduler->queue, coro);
    atomic_fetch_add(&scheduler->count_not_ready_coroutine, 1);

    return 0;
}

static void* scheduler_thread(void *arg)
{
    scheduler_t *scheduler = (scheduler_t*)arg;
    context_t *thread_ctx = context_create(empty, 0);

    if (!thread_ctx)
    {
        perror("error crete context\n");
        return NULL;
    }

    while(atomic_load(&scheduler->count_not_ready_coroutine) > 0)
    {
        void *data = scheduler_queue_pop(scheduler->queue);

        if (!data)
        {
            continue;
        }

        coroutine_t *coro = (coroutine_t*)data;

        coro->scheduler_ctx = thread_ctx;
        context_switch(thread_ctx, coro->coroutine_ctx);

        if (coro->state == FINISHED_COROUTINE)
        {
            atomic_fetch_sub(&scheduler->count_not_ready_coroutine, 1);
            coroutine_free(coro);
        }
    }

    context_free(thread_ctx);

    return NULL;
}

int scheduler_start(scheduler_t *scheduler)
{
    scheduler->status = RUNNING_SCHEDULER;

    if (!scheduler)
    {
        fprintf(stderr, "error start scheduler bot init scheduler\n");
        return -1;
    }

    scheduler->epoll_fd = epoll_create1(0);
    if (scheduler->epoll_fd == -1)
    {
        fprintf(stderr, "error epoll_create1\n");
        return -1;
    }

    for(int i = 0; i < scheduler->thread_count; i++)
    {
        pthread_create(&scheduler->threads[i], NULL, scheduler_thread, scheduler);
    }

    scheduler->status = RUNNING_SCHEDULER;
    
    while(!scheduler->stop_flag)
    {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(scheduler->epoll_fd, events, MAX_EVENTS, 100);

        if (nfds == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait\n");
            break;
        }

        for (int i = 0; i < nfds; i++) 
        {
            coroutine_t *coro = (coroutine_t*)events[i].data.ptr;
            coro->event_buffer = events[i].events;
            
            if (coro->wait_fd != -1) 
            {
                epoll_ctl(scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                coro->wait_fd = -1;
            }
            
            scheduler_queue_push(scheduler->queue, coro);
        }    

        if (atomic_load(&scheduler->count_not_ready_coroutine) == 0) {
            if (atomic_load(&scheduler->count_not_ready_coroutine) == 0) {
                scheduler->stop_flag = 1;
            }
        }
    }

    for(int i = 0; i < scheduler->thread_count; i++)
    {
        pthread_join(scheduler->threads[i], NULL);
    }

    scheduler->status = FINISHED_SCHEDULER;
    
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
