#include "libco/async_time.h"
#include "libco/scheduler.h"
#include "libco/coroutine.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_THREADS 20000
#define TIME_SLEEP 100

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void *thread_example(void *arg)
{
    (void)arg; 
    usleep(TIME_SLEEP * 1000);
    return NULL;
}

void coroutine_example(coroutine_t *coro, void *arg)
{
    (void)arg;
    async_sleep(coro, TIME_SLEEP); 
}

int main()
{
    int max_task = 1000;
    int step = 100;
    int start_task = 100;

    printf("Tasks\tThread(s)\tCoroutine(s)\t%% slower\n");

    for (int COUNT_TEST = start_task; COUNT_TEST <= max_task; COUNT_TEST += step)
    {
        int thread_count = (COUNT_TEST > MAX_THREADS) ? MAX_THREADS : COUNT_TEST;

        // --- Потоки ---
        double start = now();
        pthread_t *threads = malloc(sizeof(pthread_t) * thread_count);
        for (int i = 0; i < thread_count; i++)
            pthread_create(&threads[i], NULL, thread_example, NULL);
        for (int i = 0; i < thread_count; i++)
            pthread_join(threads[i], NULL);
        free(threads);
        double thread_res = now() - start;

        // --- Корутины ---
        start = now();
        scheduler_t *scheduler = scheduler_create();
        for (int i = 0; i < COUNT_TEST; i++)
        {
            coroutine_t *coro = coroutine_create(coroutine_example, NULL);
            scheduler_add_coroutine_to_start(scheduler, coro);
        }
        scheduler_start(scheduler);
        scheduler_free(scheduler);
        double coroutine_res = now() - start;

        double slowdown_percent = ((thread_res - coroutine_res) / coroutine_res) * 100.0;
        printf("%d\t%f\t%f\t%.2f%%\n", COUNT_TEST, thread_res, coroutine_res, slowdown_percent);
    }

    return 0;
}

