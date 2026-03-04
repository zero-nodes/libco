#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "context/context.h"
#include "coroutine.h"

typedef enum
{
    INIT_SCHEDULER,
    RUNNING_SCHEDULER,
    FINISHED_SCHEDULER
} scheduler_satus;

typedef struct scheduler_t
{
    int epoll_fd;
    int count_not_ready_coroutine;
    scheduler_satus status;
    context_t *ctx;
    coroutine_t **start_list;
    size_t len_start_list;
    size_t cap_start_list;
} scheduler_t;

extern scheduler_t* create_scheduler();
extern void free_scheduler(scheduler_t *scheduler);
extern int scheduler_add_coroutine_to_start(scheduler_t *scheduler, coroutine_t *coro);
extern int scheduler_start(scheduler_t *scheduler);
extern int scheduler_ctl_add(scheduler_t *scheduler, coroutine_t *coro, int fd, uint32_t event_type);

#endif
