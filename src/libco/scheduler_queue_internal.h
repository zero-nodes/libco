#ifndef SCHEDULER_QUEUE_H
#define SCHEDULER_QUEUE_H

#include <stddef.h>

typedef struct scheduler_queue_slot_t
{
    _Atomic size_t sequence;   
    void *data; // data
} scheduler_queue_slot_t;

typedef struct scheduler_queue_t 
{
    scheduler_queue_slot_t *data;
    _Atomic(size_t) first_idx; // push index
    _Atomic(size_t) last_idx; // pop index
    size_t cap;
} scheduler_queue_t;

extern scheduler_queue_t *scheduler_queue_create();
extern int push(scheduler_queue_t *queue, void *data);
extern void* pop(scheduler_queue_t *queue);
extern void scheduler_queue_free(scheduler_queue_t *queue);

#endif
