// Gil Elbaz 206089690

#include "threadPool.h"

typedef struct {
    void (*function)(void *);

    void *argument;
} threadpool_task_t;

static void *threadpool_thread(void *threadpool);

int threadpool_free(ThreadPool *pool);

ThreadPool *tpCreate(int numOfThreads) {
    ThreadPool *threadPool;
    int i;
    threadPool->thread_count = 0;
    threadPool->shutdown = threadPool->started = 0;

    // Allocating memory
    threadPool->threads = (pthread_t *) malloc(sizeof(pthread_t) * numOfThreads);

    // Initialize mutex
    if (pthread_mutex_init(&(threadPool->lock),NULL)!=0 ||
    (pthread_cond_init(&threadPool->update,NULL)!=0)|| (threadPool->threads == NULL)){
        goto clean;
    }

    // Check for invalid number of threads or empty
    if (numOfThreads < 0) {
        return NULL;
    }

    for (i = 0; i < numOfThreads; i++) {
        // If pool is empty
        if (pthread_create(&threadPool->threads[i], NULL, threadpool_thread, (void *) threadPool)) {
            tpDestroy(threadPool, 0);
            return NULL;
        }
        threadPool->thread_count++;
        threadPool->started = 1;
    }

    return threadPool;

    clean:
    if (threadPool){
        threadpool_free(threadPool);
    }
    return NULL;
}


void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {

}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
}



