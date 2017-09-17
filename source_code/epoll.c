#include "epoll.h"

struct epoll_event* ep_events;

int ian_epoll_create(int flags){
    int epfd = epoll_create1(flags);
    if(epfd == -1)
        return -1;
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    return epfd;
}

//注册新描述符
int ian_epoll_add(int epfd, int fd, ian_http_request_t* request, int events){
    struct epoll_event ep_event;
    ep_event.events = events;
    ep_event.data.ptr = (void *)request;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ep_event) == -1)
        return -1;
    return 0;
}

//修改epfd里的描述符
int ian_epoll_mod(int epfd, int fd, ian_http_request_t* request, int events){
    struct epoll_event ep_event;
    ep_event.events = events;
    ep_event.data.ptr = (void *)request;
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ep_event) == -1)
        return -1;
    return 0;
}

//从epfd里删除描述符
int ian_epoll_del(int epfd, int fd, ian_http_request_t* request, int events){
    struct epoll_event ep_event;
    ep_event.events = events;
    ep_event.data.ptr = (void *)request;
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ep_event) == -1)
        return -1;
    return 0;
}

//返回发生时间的文件描述符
int ian_epoll_wait(int epfd, struct epoll_event* events, int max_events, int timeout){
    return epoll_wait(epfd, events, EPOLL_SIZE, timeout);
}

void ian_handle_events(int epfd, int listenfd, struct epoll_event* events,
                        int events_num, char* path, ian_threadpool_t* pool)
{
    for(int i=0; i<events_num; i++){
        ian_http_request_t* request = (ian_http_request_t *)(ep_events[i].data.ptr);
        int fd = request->fd;

        //有事件发生的描述符是监听描述符
        if(fd == listenfd){
            accept_connection(litenfd, epfd, path);
        }
        //有事件发生的不是监听描述符
        else{
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
            || (!(events[i].events & EPOLLIN))){
                close(fd);
                continue;
            }

            threadpool_add(pool, do_request, events[i].data.ptr);
        }
    }
}