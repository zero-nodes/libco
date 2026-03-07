#include "libco/async_socket.h"
#include "libco/scheduler.h"
#include "libco/coroutine.h"

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
        fprintf(stderr, "fcntl get\n");
        close(fd);
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        fprintf(stderr, "fcntl set\n");
        close(fd);
        return -1;
    }

    return fd;
}

int async_accept(coroutine_t *coro, int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLIN | EPOLLONESHOT);
    
    while (1)
    {
        int client_fd = accept(fd, addr, addrlen);
        if (client_fd >= 0) {
            if (coro->wait_fd != -1) {
                epoll_ctl(coro->scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                coro->wait_fd = -1;
            }
            int flags = fcntl(client_fd, F_GETFL, 0);
            if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                close(client_fd);
                return -1;
            }
            return client_fd;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            coroutine_yield(coro);
        } else {
            perror("accept\n");
            return -1;
        }
    }
}

int async_connect(coroutine_t *coro, int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret = connect(fd, addr, addrlen);
    if (ret == 0) {
        return fd;
    }

    if (errno != EINPROGRESS) {
        fprintf(stderr, "error connect\n");
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
    const char *ptr = buf;
    size_t remaining = len;

    scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLOUT | EPOLLONESHOT);

    while (remaining > 0)
    {
        ssize_t n = send(fd, ptr, remaining, flags);
        if (n >= 0) {
            ptr += n;
            remaining -= n;
            if (remaining == 0) {
                if (coro->wait_fd != -1) {
                    epoll_ctl(coro->scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                    coro->wait_fd = -1;
                }
                return len;
            }
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            coroutine_yield(coro);
        }
        else
        {
            if (coro->wait_fd != -1) {
                epoll_ctl(coro->scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                coro->wait_fd = -1;
            }
            return -1;
        }
    }
    return len; 
}

size_t async_recv(coroutine_t *coro, int fd, char *buf, size_t len, int flags)
{
    scheduler_ctl_add(coro->scheduler, coro, fd, EPOLLIN | EPOLLONESHOT);
    
    while (1)
    {
        ssize_t count = recv(fd, buf, len, flags);
        if (count >= 0) {
            if (coro->wait_fd != -1) {
                epoll_ctl(coro->scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                coro->wait_fd = -1;
            }
            return count;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            coroutine_yield(coro);
        } else {
            if (coro->wait_fd != -1) {
                epoll_ctl(coro->scheduler->epoll_fd, EPOLL_CTL_DEL, coro->wait_fd, NULL);
                coro->wait_fd = -1;
            }
            return -1;
        }
    }
}
