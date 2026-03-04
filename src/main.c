#include "colib/scheduler.h"
#include "colib/coroutine.h"
#include "colib/async_time.h"

#include <stdio.h>

void coroutine_child_example(struct coroutine_t *coro, void* arg)
{
    char *str = (char *)arg;
    for (int i = 0; i < 5; i++)
    {
        printf("%s%d\n", str, i);
    }
    return;
}

void coroutine_example(struct coroutine_t *coro, void* arg)
{
    char *str = (char *)arg;
    for (int i = 0; i < 4; i++)
    {
        printf("%s%d\n", str, i);
        if (async_sleep(coro, 1000) != 0) {
            fprintf(stderr, "async_sleep failed\n");
            return;
        }
    }

    coroutine_t *coro_c = create_coroutine(coroutine_child_example, "coro_c -> ");
    scheduler_add_coroutine_to_start(coro->scheduler, coro_c);
}

int main()
{ 
    coroutine_t *coro1 = create_coroutine(coroutine_example, "coro_1 -> ");
    coroutine_t *coro2 = create_coroutine(coroutine_example, "coro_2 -> ");
    coroutine_t *coro3 = create_coroutine(coroutine_example, "coro_3 -> ");

    scheduler_t *scheduler = create_scheduler();

    scheduler_add_coroutine_to_start(scheduler, coro1);
    scheduler_add_coroutine_to_start(scheduler, coro2);
    scheduler_add_coroutine_to_start(scheduler, coro3);

    scheduler_start(scheduler);

    free_scheduler(scheduler);

    return 0;
}
