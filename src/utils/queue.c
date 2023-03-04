#include "queue.h"

#define SWAP(x, y) \
    void* tmp = x; \
    x = y;         \
    y = tmp

static size_t _parentOf(size_t i) {
    return (i - 1) / 2;
}

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

priorityQueue_t queueCreate(int (*cmpFun)(void*, void*)) {
    priorityQueue_t queue;
    queue.buffer = malloc(QUEUE_REALLOC_SIZE * sizeof(void*));
    queue.max_size = QUEUE_REALLOC_SIZE;
    queue.size = 0;
    queue.cmpFun = cmpFun;
    return queue;
}

priorityQueue_t queueDuplicate(priorityQueue_t* queue) {
    priorityQueue_t duplicate;
    duplicate.buffer = malloc(queue->max_size * sizeof(void*));
    duplicate.max_size = queue->max_size;
    duplicate.size = queue->size;
    duplicate.cmpFun = queue->cmpFun;
    memcpy(duplicate.buffer, queue->buffer, queue->max_size * sizeof(void*));
    return duplicate;
}

void queueDestroy(priorityQueue_t* queue) {
    free(queue->buffer);
}

void* queuePush(priorityQueue_t* queue, void* element) {
    if (queue->size + 1 > queue->max_size) {
        queue->max_size += QUEUE_REALLOC_SIZE;
        queue->buffer = realloc(queue->buffer, queue->max_size * sizeof(void*));
    }

    size_t node = queue->size;
    queue->buffer[queue->size++] = element;

    while (node > 0 && queue->cmpFun(queue->buffer[_parentOf(node)], queue->buffer[node])) {
        size_t parent = _parentOf(node);
        SWAP(queue->buffer[node], queue->buffer[parent]);
        node = parent;
    }

    return element;
}

void* queuePop(priorityQueue_t* queue) {
    if (queue->size == 0)
        return NULL;

    void* element = queue->buffer[0];
    queue->buffer[0] = queue->buffer[queue->size - 1];
    --queue->size;

    _bubble_down(queue, 0);
    return element;
}

void queuePrint(priorityQueue_t* queue, FILE* outFile, void (*printNodeFun)(FILE*, void*)) {
    priorityQueue_t duplicate = queueDuplicate(queue);
    while (duplicate.size > 0) {
        void* node = queuePop(&duplicate);
        printNodeFun(outFile, node);
    }
}
