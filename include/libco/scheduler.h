#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "coroutine.h"
#include "../src/libco/scheduler_queue_internal.h"

#include <pthread.h>

typedef enum
{
    INIT_SCHEDULER,
    RUNNING_SCHEDULER,
    FINISHED_SCHEDULER
} scheduler_satus;

typedef struct scheduler_t
{
    int epoll_fd;
    short stop_flag;
    _Atomic int count_not_ready_coroutine;
    scheduler_queue_t *queue;
    scheduler_satus status;
    int thread_count;
    pthread_t *threads;
} scheduler_t;

extern scheduler_t* scheduler_create(int thread_count);
extern void scheduler_free(scheduler_t *scheduler);
extern int scheduler_add_coroutine_to_start(scheduler_t *scheduler, coroutine_t *coro);
extern int scheduler_start(scheduler_t *scheduler);
extern int scheduler_ctl_add(scheduler_t *scheduler, coroutine_t *coro, int fd, uint32_t event_type);

#endif
