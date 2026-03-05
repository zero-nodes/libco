#ifndef COROUTINE_H
#define COROUTINE_H

#include "context/context.h"

typedef struct scheduler_t scheduler_t;

typedef enum 
{
    INIT_COROUTINE,
    RUNNING_COROUTINE,
    WAITTING_COROUTINE,
    FINISHED_COROUTINE
} coroutine_state;

typedef struct coroutine_t
{
    context_t *coroutine_ctx;
    scheduler_t *scheduler;
    coroutine_state state;
    void(*func)(struct coroutine_t *coro, void* arg);
    void *arg;
    uint32_t event_buffer;
    int wait_fd;

} coroutine_t;

extern coroutine_t *create_coroutine(void(*func)(coroutine_t *coro, void* arg), void* arg);
extern void coroutine_yield(coroutine_t *coro);
extern void free_coroutine(coroutine_t *coro);

#endif
