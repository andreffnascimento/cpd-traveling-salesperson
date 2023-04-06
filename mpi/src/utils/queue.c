#include "queue.h"

struct _priorityQueue {
    void** buffer;
    size_t max_size;
    size_t size;
    int (*cmpFun)(void* a, void* b);
};

static inline size_t _parentOf(size_t i) { return (i - 1) / 2; }

void _bubble_down(priorityQueue_t* queue, size_t node) {
    size_t leftChild = 2 * node + 1;
    size_t rightChild = 2 * node + 2;
    size_t i = node;

    if (leftChild < queue->size && queue->cmpFun(queue->buffer[node], queue->buffer[leftChild]))
        i = leftChild;
    if (rightChild < queue->size && queue->cmpFun(queue->buffer[i], queue->buffer[rightChild]))
        i = rightChild;

    if (i != node) {
        SWAP(queue->buffer[i], queue->buffer[node]);
        _bubble_down(queue, i);
    }
}

priorityQueue_t* queueCreate(int (*cmpFun)(void*, void*)) {
    priorityQueue_t* queue = (priorityQueue_t*)malloc(sizeof(priorityQueue_t));
    queue->buffer = malloc(QUEUE_INITIAL_SIZE * sizeof(void*));
    queue->max_size = QUEUE_INITIAL_SIZE;
    queue->size = 0;
    queue->cmpFun = cmpFun;
    return queue;
}

void queueDestroy(priorityQueue_t* queue, void (*delFun)(void*)) {
    if (delFun != NULL)
        for (size_t i = 0; i < queue->size; i++)
            delFun(queue->buffer[i]);
    free(queue->buffer);
    free(queue);
}

void* queuePeek(priorityQueue_t* queue) { return queue->buffer[0]; }

void* queuePop(priorityQueue_t* queue) {
    if (queue->size == 0)
        return NULL;

    void* element = queue->buffer[0];
    queue->buffer[0] = queue->buffer[--queue->size];
    _bubble_down(queue, 0);
    return element;
}

void* queuePush(priorityQueue_t* queue, void* element) {
    if (queue->size + 1 > queue->max_size) {
        queue->max_size = QUEUE_SIZE_MULTIPLIER(queue->max_size);
        queue->buffer = realloc(queue->buffer, queue->max_size * sizeof(void*));
    }

    size_t el = queue->size;
    queue->buffer[queue->size++] = element;
    while (el > 0 && queue->cmpFun(queue->buffer[_parentOf(el)], queue->buffer[el])) {
        size_t parent = _parentOf(el);
        SWAP(queue->buffer[el], queue->buffer[parent]);
        el = parent;
    }

    return element;
}