#ifndef ASYNC_TIME_H
#define ASYNC_TIME_H

#include "colib/coroutine.h"
#include "colib/scheduler.h"

extern int async_sleep(coroutine_t *coro, uint64_t ms);

#endif
