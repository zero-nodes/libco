#include "scheduler_queue_internal.h"

#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SCHEDULER_QUEUE_CAP
#define SCHEDULER_QUEUE_CAP 1024 * 512
#endif

#define IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

#if !IS_POWER_OF_TWO(SCHEDULER_QUEUE_CAP)
#error "SCHEDULER_QUEUE_CAP must be a power of 2"
#endif

scheduler_queue_t *scheduler_queue_create()
{
    scheduler_queue_t *queue = malloc(sizeof(scheduler_queue_t));
    if (!queue) {
        fprintf(stderr, "error get memory for queue\n");
        return NULL;
    }

    queue->cap = SCHEDULER_QUEUE_CAP;
    queue->data = malloc(sizeof(scheduler_queue_slot_t) * queue->cap);
    if (!queue->data) {
        free(queue);
        fprintf(stderr, "error get memory for queue data\n");
        return NULL;
    }

    for (size_t i = 0; i < queue->cap; i++)
        atomic_store(&queue->data[i].sequence, i);

    atomic_store(&queue->first_idx, 0);
    atomic_store(&queue->last_idx, 0);

    return queue;
}

int push(scheduler_queue_t *queue, void *data)
{
    if(!queue)
    {
        perror("error queue NULL\n");
        return -1;
    }

    while(1)
    {
        size_t pos = atomic_load(&queue->first_idx);
        scheduler_queue_slot_t *slot = &queue->data[pos & (queue->cap - 1)];
        size_t seq = atomic_load(&slot->sequence);

        size_t diff = seq - pos;

        if (diff == 0)
        {
            if (atomic_compare_exchange_weak(&queue->first_idx, &pos, pos + 1)) 
            {
                slot->data = data;
                atomic_store(&slot->sequence, pos + 1);
                return 0;
            }
        } else if (diff < 0) 
        {
            return -2;
        }
    }
}

void* pop(scheduler_queue_t *queue)
{
    while (1) 
    {
        size_t pos = atomic_load(&queue->last_idx);
        scheduler_queue_slot_t *slot = &queue->data[pos & (queue->cap - 1)];
        size_t seq = atomic_load(&slot->sequence);

        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0)
        {
            if (atomic_compare_exchange_weak(&queue->last_idx, &pos, pos + 1)) 
            {
                void *result = slot->data;
                atomic_store(&slot->sequence, pos + queue->cap); 
                return result;
            }
        } else if (diff < 0) 
        {
            return NULL; 
        }
    }
}

void scheduler_queue_free(scheduler_queue_t *queue)
{
    if (!queue)
        return;

    if (queue->data) 
        free(queue->data);
    
    free(queue);
}
