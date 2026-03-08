
# 🎯 libco - Легковесная библиотека для асинхронного программирования на основе корутин

> 📚 **Учебный проект**  
> Данный проект является демонстрационным/учебным и создан в образовательных целях для изучения принципов работы корутин, переключения контекста и асинхронного ввода-вывода.

<div align="center">
  
  ![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
  ![C23](https://img.shields.io/badge/C23-00599C?style=for-the-badge&logo=c&logoColor=white)
  ![x86--64](https://img.shields.io/badge/x86--64-0071C5?style=for-the-badge&logo=intel&logoColor=white)
  ![Clang](https://img.shields.io/badge/Clang-262D3A?style=for-the-badge&logo=llvm&logoColor=white)
  
</div>

## 📝 Описание

**libco** - это библиотека для языка C, реализующая легковесные корутины (сопрограммы) с использованием механизма переключения контекста на уровне ассемблера. Библиотека предоставляет удобный асинхронный интерфейс для работы с сетевыми операциями и таймерами, используя `epoll` под капотом.

## ✨ Особенности

- ⚡ **Аппаратное переключение контекста** - реализовано на ассемблере (x86-64) для максимальной производительности
- 🌐 **Асинхронные системные вызовы** - `socket`, `accept`, `connect`, `send`, `recv` без блокировки
- ⏰ **Таймеры** - асинхронный `sleep` с использованием `timerfd`
- 📊 **Планировщик на epoll** - эффективное управление событиями
- 🚀 **Минимальные накладные расходы** - корутины легковеснее потоков ОС
- 🎨 **Простой API** - похожий на синхронное программирование, но асинхронный внутри

## 🔧 Требования

| Компонент | Требование |
|-----------|------------|
| 🏗️ **Архитектура** | `x86-64` |
| 📋 **Стандарт языка** | `C23` |
| 🔨 **Ассемблер** | GNU as (совместим с clang) |
| 🐧 **Ядро ОС** | Linux с поддержкой `epoll` (версия 2.6 и выше) |
| 🛠️ **Компилятор** | Clang (с поддержкой C23) |

## 📦 Сборка

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

## 🚀 Использование

### 💡 Базовый пример

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

### 🔨 Сборка программы с библиотекой

```bash
clang -std=c23 -I./include -L./build your_program.c -lco -o your_program
```

## 📚 API

### 🎯 Управление контекстом

| Функция | Описание |
|---------|----------|
| `context_t *context_create(void *func, size_t count_args, ...)` | Создание контекста выполнения |
| `void context_save(context_t *ctx)` | Сохранение текущего контекста |
| `void context_switch(context_t *old_ctx, context_t *new_ctx)` | Переключение между контекстами |
| `void context_run(context_t *ctx)` | Запуск контекста |
| `void context_free(context_t *ctx)` | Освобождение ресурсов контекста |

### 🔄 Управление корутинами

| Функция | Описание |
|---------|----------|
| `coroutine_t *coroutine_create(void(*func)(coroutine_t*, void*), void *arg)` | Создание корутины |
| `void coroutine_yield(coroutine_t *coro)` | Добровольная передача управления |
| `void coroutine_free(coroutine_t *coro)` | Удаление корутины |

### 📋 Планировщик

| Функция | Описание |
|---------|----------|
| `scheduler_t *scheduler_create()` | Создание планировщика |
| `void scheduler_free(scheduler_t *scheduler)` | Удаление планировщика |
| `int scheduler_add_coroutine_to_start(scheduler_t *sched, coroutine_t *coro)` | Добавление корутины в планировщик |
| `int scheduler_start(scheduler_t *sched)` | Запуск планировщика |

### 🌐 Асинхронные сокеты

| Функция | Описание |
|---------|----------|
| `int async_socket(coroutine_t *coro, int domain, int type, int protocol)` | Создание сокета |
| `int async_accept(coroutine_t *coro, int sockfd, struct sockaddr *addr, socklen_t *addrlen)` | Принятие соединения |
| `int async_connect(coroutine_t *coro, int sockfd, const struct sockaddr *addr, socklen_t addrlen)` | Подключение к серверу |
| `size_t async_send(coroutine_t *coro, int sockfd, const void *buf, size_t len, int flags)` | Отправка данных |
| `size_t async_recv(coroutine_t *coro, int sockfd, char *buf, size_t len, int flags)` | Получение данных |

### ⏱️ Асинхронные таймеры

| Функция | Описание |
|---------|----------|
| `int async_sleep(coroutine_t *coro, uint64_t ms)` | Асинхронный сон (в миллисекундах) |

## 📁 Примеры

В директории [`examples/`](./examples) находятся примеры использования библиотеки:

| Пример | Описание |
|--------|----------|
| 🕒 `timer.c` | Демонстрация работы асинхронного таймера и создания дочерних корутин |
| ⚡ `threads_vs_coroutines_speed.c` | Сравнение производительности потоков и корутин |

## 📊 Производительность

Пример `threads_vs_coroutines_speed.c` демонстрирует значительное преимущество корутин над потоками при выполнении большого количества конкурентных операций с ожиданием. Корутины потребляют меньше памяти и создаются быстрее.

## 🤝 Вклад в проект

Буду рад любым предложениям по улучшению библиотеки! Создавайте issue или отправляйте pull requests.

---

<div align="center">
  
  **⭐ Если проект оказался полезным, не забудьте поставить звездочку! ⭐**
  
  [⬆ Вернуться к началу](#-libco---легковесная-библиотека-для-асинхронного-программирования-на-основе-корутин)
  
</div>
