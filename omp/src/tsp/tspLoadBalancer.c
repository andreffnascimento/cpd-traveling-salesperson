#include "tspLoadBalancer.h"
#include "utils/queue.h"
#include <omp.h>

typedef struct {
    bool running;
    priorityQueue_t queue;
    tspContainer_t* container;
    omp_lock_t threadLock;
    omp_lock_t threadSleep;
} tspLoadBalancerThread_t;

static void tspLoadBalancerThreadInit(tspLoadBalancerThread_t* thread, int (*cmpFun)(void*, void*)) {
    thread->running = true;
    thread->queue = queueCreate(cmpFun);
    thread->container = tspContainerCreate();
    omp_init_lock(&thread->threadLock);
    omp_init_lock(&thread->threadSleep);
}

static void tspLoadBalancerThreadDestroy(tspLoadBalancerThread_t* thread) {
    omp_destroy_lock(&thread->threadSleep);
    omp_destroy_lock(&thread->threadLock);
    tspContainerDestroy(thread->container);
    queueDelete(&thread->queue, NULL);
}

struct _tspLoadBalancer {
    int nThreads;
    int nLockedThreads;
    bool running;
    tspLoadBalancerThread_t* threads;
};

tspLoadBalancer_t* tspLoadBalancerCreate(int nThreads, int (*cmpFun)(void*, void*)) {
    tspLoadBalancer_t* tspLoadBalancer = (tspLoadBalancer_t*)malloc(sizeof(tspLoadBalancer_t));
    tspLoadBalancer->nThreads = nThreads;
    tspLoadBalancer->nLockedThreads = 0;
    tspLoadBalancer->running = true;
    tspLoadBalancer->threads = (tspLoadBalancerThread_t*)malloc(nThreads * sizeof(tspLoadBalancerThread_t));
    for (int i = 0; i < nThreads; i++)
        tspLoadBalancerThreadInit(&tspLoadBalancer->threads[i], cmpFun);
    return tspLoadBalancer;
}

void tspLoadBalancerDestroy(tspLoadBalancer_t* tspLoadBalancer) {
    for (int i = 0; i < tspLoadBalancer->nThreads; i++)
        tspLoadBalancerThreadDestroy(&tspLoadBalancer->threads[i]);
    free(tspLoadBalancer->threads);
    free(tspLoadBalancer);
}

tspContainer_t* tspLoadBalancerGetContainer(tspLoadBalancer_t* tspLoadBalancer) {
    return tspLoadBalancer->threads[omp_get_thread_num()].container;
}

void tspLoadBalancerDestroyEntry(tspLoadBalancer_t* tspLoadBalancer, tspContainerEntry_t* entry) {
    tspLoadBalancerThread_t* thread = &tspLoadBalancer->threads[omp_get_thread_num()];
#pragma omp critical(tspLoadBalancerContainer)
    tspContainerRemoveEntry(thread->container, entry);
}

void _releaseAllThreads(tspLoadBalancer_t* tspLoadBalancer) {
    for (int i = 0; i < tspLoadBalancer->nThreads; i++) {
        while (tspLoadBalancer->threads[i].running)
            ;
        omp_unset_lock(&tspLoadBalancer->threads[i].threadSleep);
    }
    tspLoadBalancer->running = false;
}

void _lockThread(tspLoadBalancer_t* tspLoadBalancer, tspLoadBalancerThread_t* thread) {
    bool terminate = true;
#pragma omp critical(tspLoadBalancer_running)
    {
        if (++tspLoadBalancer->nLockedThreads < tspLoadBalancer->nThreads)
            terminate = false;
    }

    if (!terminate) {
        omp_set_lock(&thread->threadSleep);
        thread->running = false;
        omp_unset_lock(&thread->threadLock);
        omp_set_lock(&thread->threadSleep);
        omp_unset_lock(&thread->threadSleep);
    } else {
        thread->running = false;
        _releaseAllThreads(tspLoadBalancer);
    }
}

void _unlockThread(tspLoadBalancer_t* tspLoadBalancer, tspLoadBalancerThread_t* thread) {
#pragma omp critical(tspLoadBalancer_running)
    {
        if (!thread->running) {
            tspLoadBalancer->nLockedThreads--;
            thread->running = true;
            omp_unset_lock(&thread->threadSleep);
        }
    }
}

tspContainerEntry_t* tspLoadBalancerPop(tspLoadBalancer_t* tspLoadBalancer, double branchVal) {
    tspLoadBalancerThread_t* thread = &tspLoadBalancer->threads[omp_get_thread_num()];
    omp_set_lock(&thread->threadLock);
    while (tspLoadBalancer->running) {
        tspContainerEntry_t* entry = queuePop(&thread->queue);
        if (entry == NULL || tspContainerEntryVal(entry)->lb > branchVal) {
            _lockThread(tspLoadBalancer, thread);
            continue;
        }

        omp_unset_lock(&thread->threadLock);
        return entry;
    }

    return NULL;
}

void tspLoadBalancerPush(tspLoadBalancer_t* tspLoadBalancer, const tspNode_t* parent, double cost, double lb,
                         int cityNumber) {
    static int pushIndex = 0;
    int threadNum = 0;
#pragma omp critical(tspLoadBalancer_pushIndex)
    {
        while (true) {
            pushIndex = (pushIndex + 1) % tspLoadBalancer->nThreads;
            if (omp_test_lock(&tspLoadBalancer->threads[pushIndex].threadLock)) {
                threadNum = pushIndex;
                break;
            }
        }
    }

    tspContainerEntry_t* entry;
    tspLoadBalancerThread_t* thread = &tspLoadBalancer->threads[threadNum];
#pragma omp critical(tspLoadBalancerContainer)
    entry = tspContainerGetEntry(thread->container);
    tspNodeInitExt(tspContainerEntryVal(entry), parent, cost, lb, cityNumber);
    queuePush(&thread->queue, entry);
    _unlockThread(tspLoadBalancer, thread);
    omp_unset_lock(&thread->threadLock);
}
