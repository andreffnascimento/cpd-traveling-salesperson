#include "tspLoadBalancer.h"
#include "utils/queue.h"
#include <omp.h>
#include <pthread.h>

static int __tspNodeCmpFun(void* el1, void* el2) {
    tspNode_t* tspNode1 = (tspNode_t*)el1;
    tspNode_t* tspNode2 = (tspNode_t*)el2;
    return (tspNode2->priority < tspNode1->priority ? 1 : 0);
}

static void __tspNodeDestroyFun(void* el) {
    tspNode_t* node = (tspNode_t*)el;
    tspNodeDestroy(node);
}

typedef struct {
    bool running;
    priorityQueue_t* queue;
    omp_lock_t queueLock;
    pthread_cond_t threadWait;
    pthread_mutex_t threadWaitLock;
} threadInfo_t;

threadInfo_t threadInfoCreate() {
    threadInfo_t threadInfo;
    threadInfo.running = true;
    threadInfo.queue = queueCreate(__tspNodeCmpFun);
    omp_init_lock(&threadInfo.queueLock);
    pthread_cond_init(&threadInfo.threadWait, NULL);
    pthread_mutex_init(&threadInfo.threadWaitLock, NULL);
    return threadInfo;
}

void threadInfoDestroy(threadInfo_t* threadInfo) {
    pthread_mutex_destroy(&threadInfo->threadWaitLock);
    pthread_cond_destroy(&threadInfo->threadWait);
    omp_destroy_lock(&threadInfo->queueLock);
    queueDestroy(threadInfo->queue, __tspNodeDestroyFun);
}

struct _tspLoadBalancer {
    int nThreads;
    int nStoppedThreads;
    int lastPushIndex;
    threadInfo_t* threads;
};

tspLoadBalancer_t* tspLoadBalancerCreate(int nThreads) {
    tspLoadBalancer_t* loadBalancer = (tspLoadBalancer_t*)malloc(sizeof(tspLoadBalancer_t));
    loadBalancer->threads = (threadInfo_t*)malloc(nThreads * sizeof(threadInfo_t));
    loadBalancer->nThreads = nThreads;
    loadBalancer->nStoppedThreads = 0;
    loadBalancer->lastPushIndex = 0;
    for (int i = 0; i < nThreads; i++)
        loadBalancer->threads[i] = threadInfoCreate();
    return loadBalancer;
}

void tspLoadBalancerDestroy(tspLoadBalancer_t* tspLoadBalancer) {
    for (int i = 0; i < tspLoadBalancer->nThreads; i++)
        threadInfoDestroy(&tspLoadBalancer->threads[i]);
    free(tspLoadBalancer->threads);
    free(tspLoadBalancer);
}

static bool _stopThread(tspLoadBalancer_t* tspLoadBalancer, threadInfo_t* thread) {
    bool updated = false;
#pragma omp critical(running)
    {
        if (thread->running) {
            thread->running = false;
            tspLoadBalancer->nStoppedThreads++;
            updated = true;
        }
    }
    return updated;
}

static bool _startThread(tspLoadBalancer_t* tspLoadBalancer, threadInfo_t* thread) {
    bool updated = false;
#pragma omp critical(running)
    {
        if (!thread->running) {
            thread->running = true;
            tspLoadBalancer->nStoppedThreads--;
            updated = true;
        }
    }
    return updated;
}

static void _terminate(tspLoadBalancer_t* tspLoadBalancer) {
    for (int i = 0; i < tspLoadBalancer->nThreads; i++) {
        threadInfo_t* thread = &tspLoadBalancer->threads[i];
        pthread_mutex_lock(&thread->threadWaitLock);
        thread->running = true;
        pthread_cond_signal(&thread->threadWait);
        pthread_mutex_unlock(&thread->threadWaitLock);
    }
}

static tspNode_t* _getNextNode(priorityQueue_t* queue, double solutionPriority) {
    tspNode_t* node = queuePop(queue);
    if (node != NULL && node->priority > solutionPriority) {
        tspNodeDestroy(node);
        return NULL;
    }
    return node;
}

tspNode_t* tspLoadBalancerPop(tspLoadBalancer_t* tspLoadBalancer, double* solutionPriority) {
    int threadNum = omp_get_thread_num();
    threadInfo_t* thread = &tspLoadBalancer->threads[threadNum];
    tspNode_t* node = NULL;

    while (tspLoadBalancer->nStoppedThreads < tspLoadBalancer->nThreads) {
        omp_set_lock(&thread->queueLock);
        node = (tspNode_t*)_getNextNode(thread->queue, *solutionPriority);
        omp_unset_lock(&thread->queueLock);
        if (node != NULL)
            break;

        if (_stopThread(tspLoadBalancer, thread)) {
            if (tspLoadBalancer->nStoppedThreads == tspLoadBalancer->nThreads) {
                pthread_mutex_unlock(&thread->threadWaitLock);
                _terminate(tspLoadBalancer);
                break;
            }

            pthread_mutex_lock(&thread->threadWaitLock);
            while (!thread->running)
                pthread_cond_wait(&thread->threadWait, &thread->threadWaitLock);
            pthread_mutex_unlock(&thread->threadWaitLock);
        }
    }

    return node;
}

tspNode_t* tspLoadBalancerPush(tspLoadBalancer_t* tspLoadBalancer, tspNode_t* tspNode) {
    threadInfo_t* thread = NULL;
#pragma omp critical(pushIndex)
    {
        tspLoadBalancer->lastPushIndex = (tspLoadBalancer->lastPushIndex + 1) % tspLoadBalancer->nThreads;
        thread = &tspLoadBalancer->threads[tspLoadBalancer->lastPushIndex];
    }

    omp_set_lock(&thread->queueLock);
    tspNode_t* node = (tspNode_t*)queuePush(thread->queue, tspNode);
    omp_unset_lock(&thread->queueLock);

    if (_startThread(tspLoadBalancer, thread)) {
        pthread_mutex_lock(&thread->threadWaitLock);
        pthread_cond_signal(&thread->threadWait);
        pthread_mutex_unlock(&thread->threadWaitLock);
    };
    return node;
}
