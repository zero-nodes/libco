#include "coroutine.h"
#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void coroutine_entry(coroutine_t *coro)
{
    coro->func(coro, coro->arg);

    coro->state = FINISHED_COROUTINE;
    
    context_switch(coro->coroutine_ctx, coro->scheduler->ctx);
}

coroutine_t *create_coroutine(void(*func)(coroutine_t *coro, void* arg), void* arg)
{
    coroutine_t *coro = malloc(sizeof(coroutine_t));
    if (!coro)
    {
        fprintf(stderr, "error malloc get coroutine\n");
        return NULL;
    }

    memset(coro, 0, sizeof(coroutine_t));
    
    coro->coroutine_ctx = create_context(coroutine_entry, 1, coro);
    if (!coro->coroutine_ctx)
    {
        fprintf(stderr, "error create context\n");
        free_coroutine(coro);
        return NULL;
    }

    coro->func = func;
    coro->arg = arg;
    coro->state = INIT_COROUTINE;

    return coro;
}

void free_coroutine(coroutine_t *coro)
{
    if (coro)
    {
        if (coro->coroutine_ctx)
        {
            free_context(coro->coroutine_ctx);
        }
        free(coro);
    }
}
