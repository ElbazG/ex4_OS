//
// Created by Gil on 6/15/2021.
//

#ifndef EX4_OS_THREADPOOL_H
#define EX4_OS_THREADPOOL_H
#include <pthread.h>
typedef struct ThreadPool ThreadPool;

ThreadPool* tpCreate(int numOfThreads);
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);
#endif //EX4_OS_THREADPOOL_H
