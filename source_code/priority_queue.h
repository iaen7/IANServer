#ifndef _PRIORITY_QUEUE_H
#define _PRIORITY_QUEUE_H

#include <stdlib.h>

#define IAN_PQ_DEFAULT_SIZE 10

typedef int (*ian_pq_comp_pt)(void *pi, void *pj);

typedef struct priority_queue{  //一个小顶堆
    void **pq;  //一个void* 的数组
    size_t nalloc;  //堆中已被分配的元素数
    size_t size;    //堆的总大小
    ian_pq_comp_pt comp;    //元素之间的关系
}ian_pq_t;

int ian_pq_init(ian_pq_t *ian_pq, tk_pq_comp_pt comp, size_t size);
int ian_pq_is_empty(ian_pq_t *ian_pq);
size_t ian_pq_size(ian_pq_t *ian_pq);
void *ian_pq_min(ian_pq_t *ian_pq);
int ian_pq_delmin(ian_pq_t *ian_pq);
int ian_pq_insert(ian_pq_t *ian_pq, void *item);
int ian_pq_sink(ian_pq_t *ian_pq, size_t i);