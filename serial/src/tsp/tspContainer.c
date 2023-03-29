#include "tspContainer.h"

struct tspContainerEntry {
    tspNode_t node;
    struct tspContainerEntry* prev;
    struct tspContainerEntry* next;
};

typedef struct tspSubContainer {
    tspContainerEntry_t entries[TSP_CONTAINER_SUB_SIZE];
    struct tspSubContainer* next;
} tspSubContainer_t;

struct tspContainer {
    tspSubContainer_t* firstSubContainer;
    tspSubContainer_t* lastSubContainer;
    tspContainerEntry_t* next;
};

void _initSubContainer(tspSubContainer_t* subContainer) {
    subContainer->next = NULL;
    subContainer->entries[0].prev = NULL;
    subContainer->entries[TSP_CONTAINER_SUB_SIZE - 1].next = NULL;
    for (int i = 1; i < TSP_CONTAINER_SUB_SIZE - 1; i++) {
        subContainer->entries[i].prev = &subContainer->entries[i - 1];
        subContainer->entries[i - 1].next = &subContainer->entries[i];
    }
}

tspContainer_t* tspContainerCreate() {
    tspContainer_t* container = (tspContainer_t*)malloc(sizeof(tspContainer_t));
    tspSubContainer_t* subContainer = (tspSubContainer_t*)malloc(sizeof(tspSubContainer_t));
    _initSubContainer(subContainer);
    container->firstSubContainer = subContainer;
    container->lastSubContainer = subContainer;
    container->next = &(container->firstSubContainer->entries[0]);
    return container;
}

void tspContainerDestroy(tspContainer_t* container) {
    tspSubContainer_t* subContainer = container->firstSubContainer;
    while (subContainer != NULL) {
        tspSubContainer_t* temp = subContainer;
        subContainer = subContainer->next;
        free(temp);
    }
    free(container);
}

void _createNewSubContainer(tspContainer_t* container, tspContainerEntry_t* lastEntry) {
    tspSubContainer_t* subContainer = (tspSubContainer_t*)malloc(sizeof(tspSubContainer_t));
    _initSubContainer(subContainer);
    lastEntry->next = &subContainer->entries[0];
    subContainer->entries[0].prev = lastEntry;
    container->lastSubContainer->next = subContainer;
    container->lastSubContainer = subContainer;
    container->next = &subContainer->entries[0];
}

tspContainerEntry_t* tspContainerGetEntry(tspContainer_t* container) {
    tspContainerEntry_t* entry = container->next;
    container->next = container->next->next;
    if (container->next == NULL)
        _createNewSubContainer(container, entry);
    return entry;
}

void tspContainerRemoveEntry(tspContainer_t* container, tspContainerEntry_t* entry) {
    tspContainerEntry_t* prev = entry->prev;
    tspContainerEntry_t* next = entry->next;        
    next->prev = prev;
    if (prev != NULL)
        prev->next = next;
    if (container->next->next != NULL)
        container->next->next->prev = entry;
    entry->prev = container->next;
    entry->next = container->next->next;
    container->next->next = entry;
}

tspNode_t* tspContainerEntryVal(tspContainerEntry_t* entry) { return &entry->node; }
