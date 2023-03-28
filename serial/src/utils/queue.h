#ifndef __UTILS__QUEUE_H__
#define __UTILS__QUEUE_H__

#include <stdio.h>

#define QUEUE_REALLOC_SIZE 1024

typedef struct {
    void** buffer;
    size_t max_size;
    size_t size;
    int (*cmpFun)(void* a, void* b);
} priorityQueue_t;

priorityQueue_t queueCreate(int (*cmpFun)(void*, void*));
priorityQueue_t queueDuplicate(priorityQueue_t* queue);
void queueDelete(priorityQueue_t* queue, void (*delFun)(void*));
void* queuePush(priorityQueue_t* queue, void* element);
void* queuePop(priorityQueue_t* queue);
void queuePrint(priorityQueue_t* queue, FILE* outFile, void (*printNodeFun)(FILE*, void*));

#endif //__UTILS__QUEUE_H__
