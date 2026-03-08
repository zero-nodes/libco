#include "libco/scheduler.h"
#include "libco/coroutine.h"
#include "libco/async_time.h"

#include <stdio.h>

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
}

int main()
{ 
    coroutine_t *coro1 = coroutine_create(coroutine_example, "coro_1 -> ");
    coroutine_t *coro2 = coroutine_create(coroutine_example, "coro_2 -> ");
    coroutine_t *coro3 = coroutine_create(coroutine_example, "coro_3 -> ");

    scheduler_t *scheduler = scheduler_create(1);

    scheduler_add_coroutine_to_start(scheduler, coro1);
    scheduler_add_coroutine_to_start(scheduler, coro2);
    scheduler_add_coroutine_to_start(scheduler, coro3);

    scheduler_start(scheduler);

    scheduler_free(scheduler);

    return 0;
}
