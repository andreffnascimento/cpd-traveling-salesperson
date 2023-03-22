#include "tspSolver.h"
#include <time.h>

typedef struct {
    bool running;
    priorityQueue_t queue;
    omp_lock_t queueLock;
} tspSolverThread_t;

void _threadCreate(tspSolverThread_t* thread, int (*cmpFun)(void*, void*)) {
    thread->running = true;
    thread->queue = queueCreate(cmpFun);
    omp_init_lock(&thread->queueLock);
}

void _threadDestroy(tspSolverThread_t* thread, void (*delFun)(void*)) {
    omp_destroy_lock(&thread->queueLock);
    queueDelete(&thread->queue, delFun);
}

struct _tspSolver {
    size_t nThreads;
    size_t nThreadsStoped;
    tspSolverThread_t* threads;
};

tspSolver_t* tspSolverCreate(size_t nThreads, int (*cmpFun)(void*, void*)) {
    tspSolver_t* tspSolver = (tspSolver_t*)malloc(sizeof(tspSolver_t));
    tspSolver->nThreads = nThreads;
    tspSolver->nThreadsStoped = 0;
    tspSolver->threads = (tspSolverThread_t*)malloc(nThreads * sizeof(tspSolverThread_t));
    for (size_t i = 0; i < nThreads; i++)
        _threadCreate(&tspSolver->threads[i], cmpFun);
    srand(time(NULL));
    return tspSolver;
}

void tspSolverDestroy(tspSolver_t* tspSolver, void (*delFun)(void*)) {
    for (size_t i = 0; i < tspSolver->nThreads; i++)
        _threadDestroy(&tspSolver->threads[i], delFun);
    free(tspSolver->threads);
    free(tspSolver);
}

bool tspSolverRunning(const tspSolver_t* tspSolver) { return tspSolver->nThreadsStoped != tspSolver->nThreads; }

void tspSolverStartThread(tspSolver_t* tspSolver, size_t threadNum) {
    tspSolverThread_t* thread = &tspSolver->threads[threadNum];
#pragma omp critical(tspSolverThreadStatus)
    {
        if (!thread->running)
            tspSolver->nThreadsStoped--;
        thread->running = true;
    }
}

void tspSolverStopThread(tspSolver_t* tspSolver, size_t threadNum) {
    tspSolverThread_t* thread = &tspSolver->threads[threadNum];
#pragma omp critical(tspSolverThreadStatus)
    {
        if (thread->running)
            tspSolver->nThreadsStoped++;
        thread->running = false;
    }
}

void* tspSolverIndexPop(tspSolver_t* tspSolver, size_t threadNum) {
    tspSolverThread_t* thread = &tspSolver->threads[threadNum];
    omp_set_lock(&thread->queueLock);
    void* element = queuePop(&thread->queue);
    omp_unset_lock(&thread->queueLock);
    return element;
}

void tspSolverIndexPush(tspSolver_t* tspSolver, size_t threadNum, void* element) {
    tspSolverThread_t* thread = &tspSolver->threads[threadNum];
    omp_set_lock(&thread->queueLock);
    queuePush(&thread->queue, element);
    omp_unset_lock(&thread->queueLock);
}

void tspSolverSeqPush(tspSolver_t* tspSolver, void* element) {
    static size_t tspSolverSeqThread = 0;
    size_t threadNum;
#pragma omp critical(tspSolverSeqThread)
    {
        tspSolverSeqThread = (tspSolverSeqThread +  1) % tspSolver-> nThreads;
        threadNum = tspSolverSeqThread;
    }

    tspSolverIndexPush(tspSolver, threadNum, element);
}

void tspSolverRandPush(tspSolver_t* tspSolver, void* element) {
    size_t threadNum = rand() % tspSolver->nThreads;
    tspSolverIndexPush(tspSolver, threadNum, element);
}
