#include "threadpool.h"

static int threadpool_free(ian_threadpool_t *pool);
static void* threadpool_worker(void *arg);

//释放线程池
int threadpool_free(ian_threadpool_t *pool){
    if(pool == NULL || pool->started > 0)
        return -1;
    if(pool->pthreads)
        free(pthreads);
    ian_job_t *old;
    while(pool->head->next){
        old = pool->head->next;
        pool->head->next = pool->head->next->next;
        free(old);
    }
    free(pool->head);
    return 0;
}

void *threadpool_worker(void *arg){
    if(arg == NULL)
        return NULL;
    
    ian_threadpool_t *pool = (ian_threadpool_t *) arg;
    ian_job_t *job;
    while(1){
        //给线程池加锁
        pthread_mutex_lock(&(pool->lock));

        //没有job,且未停机则阻塞
        while(pool->queue_size == 0 && !pool->shutdown)
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        
        //立即停机模式
        if(pool->shutdown == immediate_shutdown)
            break;
        else if(pool->shutdown == graceful_shutdown && pool->queue_size == 0)
            break;

        job = pool->head->next;
        //没有job则开锁
        if(job == NULL){
            pthread_mutex_unlock(&(pool->lock));
            continue;
        }

        pool->head->next = job->next;
        pool->queue_size--;
        pthread_mutex_unlock(&(pool->lock));

        //调用job里的函数
        (*(job->func))(job->arg);
        free(job);
    }
    pool->started--;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return NULL;
}

//释放线程资源
int threadpool_destory(ian_threadpool_t *pool, int graceful){
    if(pool == NULL)
        return ian_tp_invalid;
    if(pthread_mutex_lock(&(pool->lock)) != 0)
        return ian_tp_lock_fail;

    int err = 0;
    do{
        if(pool->shutdown){
            err = ian_tp_already_shutdown;
            break;
        }

        pool->shutdown = (graceful) ? graceful_shutdown : immediate_shutdown;

        if(pthread_cond_broadcase(&(pool->cond)) != 0){
            err = ian_tp_cond_broadcast;
            break;
        }

        if(pthread_mutex_unlock(&(pool->lock)) != 0){
            err = ian_tp_lock_fail;
            break;
        }

        for(int i=0; i<pool->thread_num; i++){
            if(pthread_join(pool->pthreads[i], NULL) != 0){
                err = ian_tp_thread_fail;
            }
        }
    }while(0);

    if(!err){
        pthread_mutex_destory(&(pool->lock));
        pthread_cond_destory(&(pool->cond));
        threadpool_free(pool);
    }
    return err;
}

ian_threadpool_t *threadpool_init(int thread_num){
    if((ian_threadpool_t *pool = (ian_threadpool_t*) malloc(sizeof(ian_threadpool_t))) == NULL){
        fprintf(stderr, "Error, malloc()\n");
        return NULL;
    }

    pool->thread_num = 0;
    pool->queue_size = 0;
    pool->shutdown = 0;
    pool->started = 0;
    //pthread_mutex_init(&(pool->lock), NULL);
    pool->pthreads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);

    pool->head = (ian_job_t *)malloc(sizeof(ian_job_t));
    if(pool->pthreads == NULL || pool->head == NULL){
        if(pool){
            threadpool_free(pool);
            return NULL;
        }
    }
    pool->head->func = NULL;
    pool->head->arg = NULL;
    pool->head->next = NULL;

    if(pthread_mutex_init(&(pool->lock),NULL) != 0){
        if(pool){
            threadpool_free(pool);
            return NULL;
        }
    }

    if(pthread_cond_init(&(pool->cond), NULL) != 0){
        if(pool){
            threadpool_free(pool);
            return NULL;
        }
    }

    for(int i=0; i<thread_num; i++){
        if(pthread_create(&(pool->pthreads[i]), NULL, threadpool_worker, (void*)pool) != 0){
            threadpool_destory(pool,0);
            return NULL;
        }
        pool->thread_num++;
        pool->started++;
    }
    return pool;
}

int threadpool_add(ian_threadpool_t* pool, void (*func)(void*), void *arg){
    int err =0;
    if(pool == NULL || func == NULL)
        return -1;
    
    if(pthread_mutex_lock(&(pool->lock))!=0)
        return ian_tp_lock_fail;

    if(pool->shutdown){
        err = ian_tp_already_shutdown;
        if(pthread_mutex_unlock(&(pool->lock)) !=0)
            return ian_tp_lock_fail;
        return err;
    }

    ian_job_t *job = (ian_job_t*)malloc(sizeof(ian_job_t));
    if(job == NULL){
        if(pthread_mutex_unlock(&(pool->lock)) !=0)
            return ian_tp_lock_fail;
        return err;
    }
    job->func = func;
    job->arg = arg;
    job->next = pool->head->next;
    pool->head->next = job;
    pool->queue_size++;

    if(pthread_mutex_unlock(&(pool->lock)) !=0)
            return ian_tp_lock_fail;
    pthread_cond_signal(&(pool->cond));
    return err;
}