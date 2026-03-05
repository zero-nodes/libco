#ifndef ASYNC_SOCKET_H
#define ASYNC_SOCKET_H

#include "libco/scheduler.h"
#include "libco/coroutine.h"

#include <sys/socket.h>

extern int async_socket(coroutine_t *coro, int domain, int type, int protocol);
extern int async_accept(coroutine_t *coro, int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int async_connect(coroutine_t *coro, int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern size_t async_send(coroutine_t *coro, int sockfd, const void *buf, size_t len, int flags);
extern size_t async_recv(coroutine_t *coro, int sockfd, char *buf, size_t len, int flags);

#endif
