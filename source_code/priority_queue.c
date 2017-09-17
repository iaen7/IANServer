#include <string.h>
#include "priority_queue.h"

static void swap(ian_pq_t *ian_pq, size_t i, size_t j){
    void *tmp = ian_pq->pq[i];
    ian_pq->pq[i] = ian_pq->pq[j];
    ian_pq->pq[j] = tmp;
}

static void swim(ian_pq_t *ian_pq, size_t k){
    while(k>1 && ian_pq->comp(ian_pq->pq[k], ian_pq->pq[k/2])){
        swap(ian_pq, k, k/2);
        k /= 2;
    }
}

static int sink(ian_pq_t *ian_pq, size_t k){
    size_t j;
    size_t nalloc = ian_pq->nalloc;
    while((k<<1) <= nalloc){
        j = k << 1;
        if((j<nalloc) && (ian_pq->comp(ian_pq->pq[j+1], ian_pq->pq[j])))
            j++;
        if(!ian_pq->comp(ian_pq->pq[j], ian_pq->pq[k]))
            break;
        
        swap(ian_pq, j,k);
        k = j;
    }
    return k;
}

int ian_pq_sink(ian_pq_t *ian_pq, size_t i){
    return sink(ian_pq, i);
}

int ian_pq_init(ian_pq_t *ian_pq, ian_pq_comp_pt comp, size_t size){
    ian_pq->pq = (void **)malloc(sizeof(void *) * (size+1));
    if(!ian_pq->pq)
        return -1;
    ian_pq ->nalloc = 0;
    ian_pq->size = size +1;
    ian_pq->comp = comp;
    return 0;
}

int ian_pq_is_empty(ian_pq_t *ian_pq){
    return (ian_pq->nalloc == 0) ? 1:0;
}

size_t ian_pq_size(ian_pq_t *ian_pq){
    return ian_pq->nalloc;
}

void *ian_pq_min(ian_pq_t *ian_pq){
    if(ian_pq_is_empty(ian_pq))
        return (void *)(-1);

    return ian_pq->pq[1];
}

int resize(ian_pq_t *ian_pq, size_t new_size){
    if(new_size <= ian_pq->nalloc)
        return -1;
    
    void **new_ptr = (void **)malloc(sizeof(void *) * new_size);
    if(!new_ptr)
        return -1;
    
    memcpy(new_ptr, ian_pq->pq, sizeof(void *) * (ian_pq->nalloc + 1));

    free(ian_pq->pq);

    ian_pq->pq = new_ptr;
    ian_pq->size = new_size;
    return 0;
}

int ian_pq_delmin(ian_pq_t *ian_pq){
    if(ian_pq_is_empty(ian_pq))
        return 0;

    swap(ian_pq, 1, ian_pq->nalloc);
    --ian_pq->nalloc;
    sink(ian_pq,1);
    if((ian_pq->nalloc > 0) && (ian_pq->nalloc <= (ian_pq->size-1)/4)){
        if(resize(ian_pq, ian_pq->size/2) < 0)
            return -1;
    }
    return 0;
}

int ian_pq_insert(ian_pq_t *ian_pq, void *item){
    if(ian_pq->nalloc + 1==ian_pq->size){
        if(resize(ian_pq, ian_pq->size * 2) <0)
            return -1;
    }
    ian_pq->pq[++ian_pq->nalloc] = item;
    swim(ian_pq, ian_pq->nalloc);
    return 0;
}