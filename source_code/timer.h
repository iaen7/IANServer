#ifndef _IAN_TIMER_H
#define _IAN_TIMER_H

#include "priority_queue.h"
#include "http_request.h"

#define TIMEOUT_DEFAULT 500

typedef int (*timer_handler_func)(ian_http_request_t * request);
typedef struct ian_timer{
    size_t key; //超时时间
    int deleted;    //是否被删除
    timer_handler_func handler; //超时处理，添加时指定
    ian_http_request_t* request;
}ian_timer_t;

extern ian_pq_t ian_timer;
extern size_t ian_current_msec;

int ian_timer_init();
int ian_find_timer();
void ian_handle_expire_timers();
void ian_add_timer(ian_http_request_t* request, size_t timeout, timer_handler_func handler);
void ian_del_timer(ian_http_request_t* request);
int timer_comp(void *ti, void *tj);

#endif