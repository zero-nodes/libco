#ifndef ASYNC_TIME_H
#define ASYNC_TIME_H

#include "libco/coroutine.h"
#include "libco/scheduler.h"

extern int async_sleep(coroutine_t *coro, uint64_t ms);

#endif
