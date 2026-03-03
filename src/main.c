#include "context/context.h"
#include <stdio.h>
#include <stdlib.h>

void f1(void* i1, void* i2, void* i3, void* ctx, void* main_ctx)
{
    printf("%d\n", *(int*)i1);
    printf("%d\n", *(int*)i2);
    printf("%d\n", *(int*)i3);
    context_switch(*(context_t**)ctx, (context_t*)main_ctx);
}

int main()
{ 
    int* i = malloc(sizeof(int));
    *i = 1;

    context_t *main_ctx = create_context(NULL, 0);
    context_t *ctx;

    ctx = create_context(f1, 5, i, i, i, &ctx, main_ctx);
    context_save(main_ctx);
    context_run(ctx);

    printf("return main\n");

    free_context(ctx);
    free_context(main_ctx);

    return 0;
}
