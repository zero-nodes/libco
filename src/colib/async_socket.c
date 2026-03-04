#include "async_socket.h"
#include "colib/scheduler.h"
#include "colib/coroutine.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>

int async_socket(coroutine_t *coro, int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr, "fcntl get");
        close(fd);
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr, "fcntl set");
        close(fd);
        return -1;
    }

    return fd;
}

int async_accept(coroutine_t *coro, int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    int client_fd;

    while (1)
    {
        client_fd = accept(fd, addr, addrlen);
        if (client_fd >= 0) {
            break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLET | EPOLLONESHOT);
            coroutine_yield(coro);
        } else {
            perror("accept");
            return -1;
        }
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1) {
        fprintf(stderr, "fcntl get");
        close(client_fd);
        return -1;
    }

    if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr, "fcntl set");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

int async_connect(coroutine_t *coro, int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = connect(fd, addr, addrlen);
    if (ret == 0) {
        return fd;
    }

    if (errno != EINPROGRESS) {
        fprintf(stderr, "error connect");
        return -1;
    }

    scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLOUT | EPOLLONESHOT);
    coroutine_yield(coro);

    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        fprintf(stderr, "error getsockopt: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (err != 0) {
        fprintf(stderr, "error async connect: %s\n", strerror(err));
        close(fd);
        return -1;
    }

    return fd;
}

size_t async_send(coroutine_t *coro, int fd, const void *buf, size_t len, int flags)
{
    size_t n;
    const char *ptr = buf;
    size_t remaining = len;

    while(remaining > 0)
    {
        n = send(fd, ptr, remaining, flags);    
        if (n >= 0)
        {
            ptr += n;   
            remaining -= n;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLOUT | EPOLLONESHOT);
            coroutine_yield(coro);
        } 
        else
        {
            fprintf(stderr, "error send: %s\n", strerror(errno));
            return -1;
        }
    }

    return len;
}

size_t async_recv(coroutine_t *coro, int fd, char *buf, size_t len, int flags)
{
    size_t count_get_data;

    while(1)
    {
        count_get_data = recv(fd, buf, len, flags);

        if (count_get_data >= 0)
        {
            return count_get_data;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLIN | EPOLLONESHOT);
            coroutine_yield(coro); 
        } 
        else 
        {
            fprintf(stderr, "error recv: %s\n", strerror(errno));
            return -1;
        }
    }
}
