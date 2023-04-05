#ifndef __UTILS__QUEUE_H__
#define __UTILS__QUEUE_H__

#include "include.h"

typedef struct _priorityQueue priorityQueue_t; 

priorityQueue_t* queueCreate(int (*cmpFun)(void*, void*));
void queueDestroy(priorityQueue_t* queue, void (*delFun)(void*));
void* queuePeek(priorityQueue_t* queue);
void* queuePop(priorityQueue_t* queue);
void* queuePush(priorityQueue_t* queue, void* element);

#endif //__UTILS__QUEUE_H__
