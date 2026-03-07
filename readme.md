# libco - Легковесная библиотека для асинхронного программирования на основе корутин

> **Примечание:** Данный проект является демонстрационным/учебным и создан в образовательных целях для изучения принципов работы корутин, переключения контекста и асинхронного ввода-вывода.

libco - это библиотека для языка C, реализующая легковесные корутины (сопрограммы) с использованием механизма переключения контекста на уровне ассемблера. Библиотека предоставляет удобный асинхронный интерфейс для работы с сетевыми операциями и таймерами, используя epoll под капотом.

## Особенности

- **Аппаратное переключение контекста** - реализовано на ассемблере (x86-64) для максимальной производительности
- **Асинхронные системные вызовы** - socket, accept, connect, send, recv без блокировки
- **Таймеры** - асинхронный sleep с использованием timerfd
- **Планировщик на epoll** - эффективное управление событиями
- **Минимальные накладные расходы** - корутины легковеснее потоков ОС
- **Простой API** - похожий на синхронное программирование, но асинхронный внутри

## Требования

- **Архитектура:** x86-64
- **Стандарт языка:** C23
- **Ассемблер:** GNU as (совместим с clang)
- **Ядро ОС:** Linux с поддержкой epoll (версия 2.6 и выше)
- **Компилятор:** Clang (с поддержкой C23)

## Сборка

```bash
# Клонирование репозитория
git clone https://github.com/zero-nodes/libco
cd libco

# Сборка библиотеки
make

# Очистка собранных файлов
make clean
```

После сборки в директории `build/` появится статическая библиотека `libco.a`.

## Использование

### Базовый пример

```c
#include <stdio.h>
#include <libco/coroutine.h>
#include <libco/scheduler.h>
#include <libco/async_time.h>

void example_coroutine(coroutine_t *coro, void *arg)
{
    const char *name = (const char*)arg;
    
    for (int i = 0; i < 3; i++) {
        printf("%s: итерация %d\n", name, i);
        async_sleep(coro, 1000); // Асинхронный сон на 1 секунду
    }
    
    printf("%s: завершена\n", name);
}

int main()
{
    // Создание планировщика
    scheduler_t *sched = scheduler_create();
    
    // Создание корутин
    coroutine_t *coro1 = coroutine_create(example_coroutine, "Корутина 1");
    coroutine_t *coro2 = coroutine_create(example_coroutine, "Корутина 2");
    
    // Добавление корутин в планировщик
    scheduler_add_coroutine_to_start(sched, coro1);
    scheduler_add_coroutine_to_start(sched, coro2);
    
    // Запуск планировщика
    scheduler_start(sched);
    
    // Очистка
    scheduler_free(sched);
    
    return 0;
}
```

### Сборка программы с библиотекой

```bash
clang -std=c23 -I./include -L./build your_program.c -lco -o your_program
```

## API

### Управление контекстом

```c
context_t *context_create(void *func, size_t count_args, ...);
void context_save(context_t *ctx);
void context_switch(context_t *old_ctx, context_t *new_ctx);
void context_run(context_t *ctx);
void context_free(context_t *ctx);
```

### Управление корутинами

```c
coroutine_t *coroutine_create(void(*func)(coroutine_t*, void*), void *arg);
void coroutine_yield(coroutine_t *coro);
void coroutine_free(coroutine_t *coro);
```

### Планировщик

```c
scheduler_t *scheduler_create();
void scheduler_free(scheduler_t *scheduler);
int scheduler_add_coroutine_to_start(scheduler_t *sched, coroutine_t *coro);
int scheduler_start(scheduler_t *sched);
```

### Асинхронные сокеты

```c
int async_socket(coroutine_t *coro, int domain, int type, int protocol);
int async_accept(coroutine_t *coro, int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int async_connect(coroutine_t *coro, int sockfd, const struct sockaddr *addr, socklen_t addrlen);
size_t async_send(coroutine_t *coro, int sockfd, const void *buf, size_t len, int flags);
size_t async_recv(coroutine_t *coro, int sockfd, char *buf, size_t len, int flags);
```

### Асинхронные таймеры

```c
int async_sleep(coroutine_t *coro, uint64_t ms);
```

## Примеры

В директории `examples/` находятся примеры использования библиотеки:

- `timer.c` - демонстрация работы асинхронного таймера и создания дочерних корутин
- `threads_vs_coroutines_speed.c` - сравнение производительности потоков и корутин

