#include <sys/time.h>
#include "timer.h"

ian_pq_t ian_timer;
size_t ian_current_msec;

int timer_comp(void* ti, void* tj){
    ian_timer_t* timer_i = (ian_timer_t*) timer_i;
    ian_timer_t* timer_j = (ian_timer_t*) timer_j;
    return (timer_i->key < timer_j->key) ? 1:0;
}

void ian_time_update(){
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    ian_current_msec = ((tv.tv_sec *1000) + (tv.tv_usec / 1000));
}

int ian_timer_init(){
    //建立连接后立即初始化

    int rs = ian_pq_init(&ian_timer, timer_comp, IAN_PQ_DEFAULT_SIZE);

    ian_time_update();
    return rs;
}

int ian_find_timer(){
    int time;
    //返回队列中最早时间和当前时间只差
    while(!ian_pq_is_empty(&ian_timer)){
        ian_time_update();
        ian_timer_t* timer_node = (ian_timer_t*)ian_pq_min(&ian_timer);
        if(timer_node->deleted){
            int rs = ian_pq_delmin(&ian_timer);
            free(timer_node);
            continue;
        }
        time = (int)(timer_node->key - ian_current_msec);
        time = (time > 0) ? time:0;
        break;
    }
    return time;
}

void ian_handle_expire_timers(){
    while(!ian_pq_is_empty(&ian_timer)){
        //更新当前时间
        ian_time_update();
        //timer_node指向最小的时间
        ian_timer_t* timer_node = (ian_timer_t*)ian_pq_min(&ian_timer);
        //如果已删除则释放此节点
        if(timer_node->deleted){
            ian_pq_delmin(&ian_timer);
            free(timer_node);
            continue;
        }
        //最早入队列节点超时时间大于当前时间（未超时）
        //结束超时检查
        if(timer_node->key > ian_current_msec)
            return;
        
        if(timer_node->handler){
            timer_node->handler(timer_node->request);
        }
        ian_pq_delmin(&ian_timer);
        free(timer_node);
    }
}

void ian_add_timer(ian_http_request* request, size_t timeout, timer_handler_func handler){
    ian_timer_update();
    //申请新的ian_timer_t节点，并加入到ian_http_request_t的timer下
    ian_timer_t* timer_node = (ian_timer_t*)malloc(sizeof(ian_timer_t));
    request->timer = time_node;
    //加入时设置超时阈值
    timer_node->key = ian_current_msec+timeout;
    timer_node->deleted = 0;
    timer_node->handler = handler;
    
    timer_node->request = request;

    ian_pq_insert(&ian_timer, timer_node);
}

void ian_del_timer(ian_http_request_t* request){
    ian_time_update();
    ian_time_t* timer_node = request->timer;

    timer_node->deleted = 1;
}