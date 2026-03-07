#include "context/context.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef STANDARD_STACK_CONTEXT_SIZE;
#define STANDARD_STACK_CONTEXT_SIZE (64 * 1024)
#endif

context_t *context_create(void *func, size_t count_args, ...)
{
    regs_t *init_regs;
    context_t *ctx;

    ctx = malloc(sizeof(*ctx));
    if (!ctx) 
    {
        fprintf(stderr, "error malloc get ctx\n");
        return NULL;
    }
    ctx->stack = (uintptr_t)malloc(STANDARD_STACK_CONTEXT_SIZE);
    if (!ctx->stack) 
    {
        fprintf(stderr, "error malloc get stack\n");
        free(ctx); 
        return NULL;
    }
    
    ctx->stack_sp = (uintptr_t)ctx->stack + (STANDARD_STACK_CONTEXT_SIZE - sizeof(regs_t) - 8);
    init_regs = (regs_t*)ctx->stack_sp;
    init_regs->rip = (uint64_t)func;

    init_regs->rdi = 0;
    init_regs->rsi = 0;
    init_regs->rdx = 0;
    init_regs->rcx = 0;
    init_regs->r9 = 0;

    va_list args;
    va_start(args, count_args);

    for (size_t i = 0; i < count_args; i++) {
        uint64_t value = va_arg(args, uint64_t);

        switch (i) {
            case 0: init_regs->rdi = value; break;
            case 1: init_regs->rsi = value; break;
            case 2: init_regs->rdx = value; break;
            case 3: init_regs->rcx = value; break;
            case 4: init_regs->r8  = value; break;
            case 5: init_regs->r9  = value; break;
        }
    }

    va_end(args);

    init_regs->rbx = 0;
    init_regs->rbp = 0;
    init_regs->r12 = 0;
    init_regs->r13 = 0;
    init_regs->r14 = 0;
    init_regs->r15 = 0;

    return ctx;
}

void context_free(context_t *ctx)
{
    if (ctx)
    {
        if (ctx->stack)
        {
            free((void*)ctx->stack);
        }
        free((void*)ctx);
    }
}
