// Gil Elbaz 206089690

#include "threadPool.h"

void *function(void *task) {
    ThreadPool *threadPool;
    threadPool = (ThreadPool *) task;
    threadPool->perform_task(task);
    return NULL;
}


void working(ThreadPool *threadPool, int flag) {
    if (flag != WORKS) {
        perror("Error in system call");
        tpDestroy(threadPool, 0);
        _exit(-1);
    }
}

void perform_task(void *tasks) {
    int status;
    // Checking for empty pool
    ThreadPool *threadPool = (ThreadPool *) tasks;
    if (threadPool == NULL) {
        perror("Error in system call");
        exit(1);
    }

    while (!threadPool->should_destroy) {
        // Locking critical section
        status = pthread_mutex_lock(&threadPool->mutex_task_queue);
        working(threadPool, status);

        if (osIsQueueEmpty(threadPool->tasks_queue) && (!threadPool->should_destroy)) {
            status = pthread_mutex_lock((&threadPool->mutex_empty));
            working(threadPool, status);

            if (threadPool->should_empty != START) {
                status = pthread_mutex_unlock(&threadPool->mutex_empty);
                working(threadPool, status);
                status = pthread_mutex_unlock(&threadPool->mutex_task_queue);
                working(threadPool, status);
                break;
            }
            status = pthread_mutex_unlock(&threadPool->mutex_empty);
            working(threadPool, status);

            status = pthread_cond_wait(&threadPool->condition, &threadPool->mutex_task_queue);
            working(threadPool, status);
        }

        status = pthread_mutex_unlock(&threadPool->mutex_task_queue);
        working(threadPool, status);

        status = pthread_mutex_lock(&threadPool->mutex_queue);
        working(threadPool, status);

        if (!osIsQueueEmpty(threadPool->tasks_queue)) {
            Job *job = (Job *) osDequeue(threadPool->tasks_queue);
            if (job == NULL) {
                continue;
            }
            status = pthread_mutex_unlock(&threadPool->mutex_queue);
            working(threadPool, status);
            job->func(job->args);
            free(job);
        } else {
            status = pthread_mutex_unlock(&threadPool->mutex_queue);
            working(threadPool, status);
        }

        status = pthread_mutex_lock(&threadPool->mutex_empty);
        working(threadPool, status);

        if (threadPool->should_empty == FALSE) {
            status = pthread_mutex_unlock(&threadPool->mutex_empty);
            working(threadPool, status);
            break;
        }
        status = pthread_mutex_unlock(&threadPool->mutex_empty);
        working(threadPool, status);

    }
}

void intilaize_structs(ThreadPool *threadPool, int threads) {
    threadPool->tasks_queue = osCreateQueue();
    threadPool->thread_count = threads;
    threadPool->perform_task = perform_task;
    threadPool->should_destroy = FALSE;
    threadPool->stop = FALSE;
    threadPool->should_empty = START;

    // Initialize mutex
    if (pthread_mutex_init(&threadPool->mutex_task_queue, NULL) ||
        pthread_mutex_init(&threadPool->mutex_empty, NULL), pthread_cond_init(&threadPool->condition, NULL) ||
                                                            pthread_mutex_init(&threadPool->mutex_queue, NULL)) {
        perror("Error in system call");
        free(threadPool);
        _exit(-1);
    }

    // Initialize threads
    threadPool->threads = malloc(sizeof(pthread_t) * threads);
    if (threadPool->threads == NULL) {
        perror("Error in system call");
        free(threadPool);
        _exit(-1);
    }

    int i;
    for (i = 0; i < threads; i++) {
        if (pthread_create(&(threadPool->threads[i]), NULL, function, threadPool) != 0) {
            tpDestroy(threadPool, 0);
            perror("Error in system call");
            _exit(-1);
        }
    }
}

void wait(void *arg) {
    int i;
    ThreadPool *threadPool = (ThreadPool *) arg;
    for (i = 0; i < threadPool->thread_count; i++) {
        if (pthread_join(threadPool->threads[i], NULL) != 0) {
            perror("Error in system call");
            _exit(-1);
        }
    }
}

int check_task_queue(ThreadPool *threadPool) {
    int status;
    status = pthread_mutex_lock(&threadPool->mutex_task_queue);
    working(threadPool, status);

    int empty = osIsQueueEmpty(threadPool->tasks_queue);

    status = pthread_mutex_unlock(&threadPool->mutex_task_queue);
    working(threadPool, status);
    return empty;
}

Job *fetch_job(ThreadPool *threadPool) {
    int status;
    status = pthread_mutex_lock(&threadPool->mutex_task_queue);
    working(threadPool, status);

    Job *job = (Job *) osDequeue(threadPool->tasks_queue);

    status = pthread_mutex_unlock(&threadPool->mutex_task_queue);
    working(threadPool, status);
    return job;
}

ThreadPool *tpCreate(int numOfThreads) {
    ThreadPool *threadPool = malloc(sizeof(ThreadPool));
    intilaize_structs(threadPool, numOfThreads);
    return threadPool;
}


void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    if (threadPool->should_destroy) {
        return;
    }
    threadPool->stop = TRUE;

    if (pthread_mutex_lock(&threadPool->mutex_task_queue) != 0) {
        perror("Error in system call");
        _exit(-1);
    }
    if (pthread_cond_broadcast(&threadPool->condition) != 0) {
        perror("Error in system call");
    }
    if (pthread_mutex_unlock(&threadPool->mutex_task_queue) != 0) {
        perror("Error in system call");
        _exit(-1);
    }
    if (shouldWaitForTasks == 1) {
        if (pthread_mutex_lock(&threadPool->mutex_empty) != 0) {
            perror("Error in system call");
            _exit(-1);
        }
        threadPool->should_empty = TRUE;

        if (pthread_mutex_unlock(&threadPool->mutex_empty) != 0) {
            perror("Error in system call");
            _exit(-1);
        }
    } else {
        if (pthread_mutex_lock(&threadPool->mutex_empty) != 0) {
            perror("Error in system call");
            _exit(-1);
        }
        threadPool->should_empty = FALSE;

        if (pthread_mutex_unlock(&threadPool->mutex_empty) != 0) {
            perror("Error in system call");
            _exit(-1);
        }
    }
    // If we should remove all from queue
    if (threadPool->should_empty) {
        wait(threadPool);
        threadPool->should_destroy = TRUE;

        while (!check_task_queue(threadPool)) {
            free(fetch_job(threadPool));
        }
        if (pthread_mutex_lock(&threadPool->mutex_task_queue) != 0) {
            perror("Error in system call");
        }
        osDestroyQueue(threadPool->tasks_queue);
        threadPool->tasks_queue = NULL;
        if (pthread_mutex_unlock(&threadPool->mutex_task_queue) != 0) {
            perror("Error in system call");
        }
    } else {
        wait(threadPool);
        threadPool->should_destroy = TRUE;
    }
    // Free all resources
    pthread_cond_destroy(&threadPool->condition);
    pthread_mutex_destroy(&threadPool->mutex_queue);
    pthread_mutex_destroy(&threadPool->mutex_empty);
    pthread_mutex_destroy(&threadPool->mutex_task_queue);
    free(threadPool->tasks_queue);
    free(threadPool->threads);
    free(threadPool);
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    if (threadPool->should_destroy || threadPool->stop) {
        return -1;
    }

    Job *job = malloc(sizeof(Job));
    if (job == NULL) {
        perror("Error in system call");
    }
    job->args = param;
    job->func = computeFunc;

    int status = pthread_mutex_lock(&threadPool->mutex_queue);
    working(threadPool, status);

    osEnqueue(threadPool->tasks_queue, job);

    status = pthread_mutex_unlock(&threadPool->mutex_queue);
    working(threadPool, status);

    status = pthread_cond_signal(&threadPool->condition);
    working(threadPool, status);

    return WORKS;
}



