#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

typedef struct ian_job{
    void* (*callback_function) (void* arg);  //线程回调函数
    void* arg;      //回调函数参数
    struct ian_job *next;
}ian_job_t;

typedef struct threadpool{
    int thread_num;  //  线程池中开启的线程个数
    int queue_size;  // 任务链表长
    ian_job_t *head;    //任务链表头
    pthread_t *pthreads;    //线程
    pthread_mutex_t lock;  //互斥锁
    pthread_cond_t cond; //条件变量
    int shutdown;
    int started;
} ian_threadpool_t;

typedef enum{
    ian_tp_invalid = -1,
    ian_tp_lock_fail = -2,
    ian_tp_already_shutdown = -3,
    ian_tp_cond_broadcast = -4,
    ian_tp_thread_fail = -5
} ian_threadpool_error_t;

typedef enum{
    immediate_shutdown = 1,
    graceful_shutdown = 2
} ian_threadpool_sd_t;

ian_threadpool_t* threadpool_init(int thread_num);
int threadpool_add(ian_threadpool_t* pool, void (*func)(void *), void* arg);
int threadpool_destroy(ian_threadpool_t* pool, int graceful);

#endif