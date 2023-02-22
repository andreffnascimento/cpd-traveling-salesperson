#ifndef __UTILS_QUEUE_H__
#define __UTILS_QUEUE_H__

#include "include.h"

#define QUEUE_REALLOC_SIZE 1024

typedef struct {
    void** buffer;
    size_t max_size;
    size_t size;
    int (*cmpFun)(void* a, void* b);
} priorityQueue_t;

priorityQueue_t queueCreate(int (*cmpFun)(void*, void*));

void queueDelete(priorityQueue_t* queue);

void* queuePush(priorityQueue_t* queue, void* element);

void* queuePop(priorityQueue_t* queue);

priorityQueue_t queueDuplicate(priorityQueue_t* queue);

void queuePrint(priorityQueue_t* queue, FILE* outFile, void (*printNodeFun)(FILE*, void*));

#endif //__UTILS_QUEUE_H__