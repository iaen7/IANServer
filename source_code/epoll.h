#ifndef _EPOLL_H
#define _EPOLL_H

#include <sys/epoll.h>
#include "http.h"
#include "threadpool.h"

#define EPOLL_SIZE 1024

int ian_epoll_create(int flags);
int ian_epoll_add(int epfd, int fd, ian_http_request_t* request, int events);
int ian_epoll_mod(int epfd, int fd, ian_http_request_t* request, int events);
int ian_epoll_del(int epfd, int fd, ian_http_request_t* request, int events);
int ian_epoll_wait(int epfd, struct epoll_event *events, int max_events, int timeout);
void ian_handle_events(int epfd, int listenfd, struct epoll_event* events,
                        int events_num, char* path, ian_threadpool_t* pool);

#endif