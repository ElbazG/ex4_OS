#include <stdio.h>
#include "threadPool.h"

ThreadPool *tpCreate(int numOfThreads) {
    return NULL;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {

}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    return 0;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}

