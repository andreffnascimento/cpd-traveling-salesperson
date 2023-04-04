#include "container.h"

struct ContainerEntry {
    Node_t node;
    struct ContainerEntry* prev;
    struct ContainerEntry* next;
};

typedef struct subContainer {
    ContainerEntry_t entries[CONTAINER_SUB_SIZE];
    struct subContainer* next;
} subContainer_t;

struct Container {
    subContainer_t* firstSubContainer;
    subContainer_t* lastSubContainer;
    ContainerEntry_t* next;
};

void _initSubContainer(subContainer_t* subContainer) {
    subContainer->next = NULL;
    subContainer->entries[0].prev = NULL;
    subContainer->entries[CONTAINER_SUB_SIZE - 1].next = NULL;
    for (int i = 1; i < CONTAINER_SUB_SIZE - 1; i++) {
        subContainer->entries[i].prev = &subContainer->entries[i - 1];
        subContainer->entries[i - 1].next = &subContainer->entries[i];
    }
}

Container_t* containerCreate() {
    Container_t* container = (Container_t*)malloc(sizeof(Container_t));
    subContainer_t* subContainer = (subContainer_t*)malloc(sizeof(subContainer_t));
    _initSubContainer(subContainer);
    container->firstSubContainer = subContainer;
    container->lastSubContainer = subContainer;
    container->next = &(container->firstSubContainer->entries[0]);
    return container;
}

void containerDestroy(Container_t* container) {
    subContainer_t* subContainer = container->firstSubContainer;
    while (subContainer != NULL) {
        subContainer_t* temp = subContainer;
        subContainer = subContainer->next;
        free(temp);
    }
    free(container);
}

void _createNewSubContainer(Container_t* container, ContainerEntry_t* lastEntry) {
    subContainer_t* subContainer = (subContainer_t*)malloc(sizeof(subContainer_t));
    _initSubContainer(subContainer);
    lastEntry->next = &subContainer->entries[0];
    subContainer->entries[0].prev = lastEntry;
    container->lastSubContainer->next = subContainer;
    container->lastSubContainer = subContainer;
    container->next = &subContainer->entries[0];
}

ContainerEntry_t* containerGetEntry(Container_t* container) {
    ContainerEntry_t* entry = container->next;
    container->next = container->next->next;
    if (container->next == NULL)
        _createNewSubContainer(container, entry);
    return entry;
}

void containerRemoveEntry(Container_t* container, ContainerEntry_t* entry) {
    ContainerEntry_t* prev = entry->prev;
    ContainerEntry_t* next = entry->next;        
    next->prev = prev;
    if (prev != NULL)
        prev->next = next;
    if (container->next->next != NULL)
        container->next->next->prev = entry;
    entry->prev = container->next;
    entry->next = container->next->next;
    container->next->next = entry;
}

Node_t* containerGetNode(ContainerEntry_t* entry) { return &entry->node; }
