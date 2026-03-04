#include "async_time.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>

int async_sleep(coroutine_t *coro, uint64_t ms)
{
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1) {
        fprintf(stderr, "error create time_fd");
        return -1;
    }
    
    struct itimerspec ts = {
        .it_interval = {0, 0},
        .it_value = {
            .tv_sec = ms / 1000,
            .tv_nsec = (ms % 1000) * 1000000
        }
    };
    timerfd_settime(timer_fd, 0, &ts, NULL);

    scheduler_ctl_add(coro->scheduler, coro, timer_fd, EPOLLIN | EPOLLONESHOT);

    coroutine_yield(coro);

    close(timer_fd);

    return 0;
}
