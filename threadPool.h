// Gil Elbaz 206089690

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#define FALSE 0
#define TRUE 1
#define WORKS 0
#define START 2

#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include "pthread.h"
#include "osqueue.h"

typedef struct thread_pool {
    // mutex variables
    pthread_mutex_t mutex_task_queue;
    pthread_mutex_t mutex_queue;
    pthread_mutex_t mutex_empty;
    pthread_cond_t condition;

    pthread_t *threads;
    OSQueue *tasks_queue;

    void (*perform_task)(void *);

    // thread counter
    int thread_count;

    int should_destroy;
    int should_empty;
    int stop;
} ThreadPool;

typedef struct {
    void (*func)(void *);
    void *args;
} Job;


ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
