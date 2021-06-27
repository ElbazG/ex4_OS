// Gil Elbaz 206089690

#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include <malloc.h>
#include "pthread.h"
#include "osqueue.h"

typedef struct thread_pool {
    pthread_mutex_t lock;
    pthread_cond_t update;
    pthread_t *threads;
    OSQueue *queue;
    int thread_count;
    int queue_size;
    int shutdown;
    int started;
} ThreadPool;

ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
