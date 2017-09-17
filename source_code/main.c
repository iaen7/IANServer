#include <stdio.h>
#include "threadpool.h"
#include "http.h"

#define DEFAULT_CONFIG "ianserver.conf"

extern struct epoll_event *events;
char *conf_file = DEFAULT_CONFIG;
ian_conf_t conf;

int main(int argc, char **argv){
    //读取配置文件
    read_conf(conf_file, &conf);

    //处理SIGPIPE信号，让服务端忽略SIGPIPE
    handle_for_sigpipe();

    //初始化套接字开始监听
    int listenfd = open_listenfd(conf.port);

    set_non_blocking(listenfd);

    int epfd = ian_epoll_create(0);
    ian_http_request_t* request = (ian_http_request_t*)malloc(sizeof(ian_http_request_t));
    ian_init_request_t(request,listenfd,epfd,conf.root);
    ian_epoll_add(epfd, listenfd, request, (EPOLLIN|EPOLLET));

    //初始化线程池
    ian_threadpool_t *pool = threadpool_init(conf.thd_num);

    ian_timer_init();

    while(1){
        int time = ian_find_timer();

        int events_num = ian_epoll_wait(epfd, events, MAXEVENTS, -1);

        ian_handle_expire_timers();

        ian_handle_events(epfd, litenfd, events, events_num, conf.root, pool);
    }
}