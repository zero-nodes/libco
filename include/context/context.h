#ifndef CONTEXT_H
#define CONTEXT_H

#include <stddef.h>
#include <stdint.h>

typedef struct regs_t
{
    uintptr_t rdi;
    uintptr_t rsi;  
    uintptr_t rdx;  
    uintptr_t rcx;  
    uintptr_t r8;
    uintptr_t r9;

    uintptr_t rbx;
    uintptr_t rbp;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;

    uintptr_t rip;
} regs_t;

typedef struct context_t
{
    uint64_t stack_sp;
    uintptr_t stack;
} context_t;

extern void context_save(context_t *ctx);
extern void context_switch(context_t *old_ctx, context_t *new_ctx);
extern void context_run(context_t *ctx);

extern context_t *context_create(void *func, size_t count_args, ...);
extern void context_free(context_t *ctx);

#endif
