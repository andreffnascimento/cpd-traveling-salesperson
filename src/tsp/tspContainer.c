#include "tspContainer.h"

tspContainer_t* tspContainerCreate() {
    tspContainer_t* tspContainer = (tspContainer_t*)malloc(sizeof(tspContainer_t));
    tspContainer->curr = 0;
    tspContainer->next = NULL;
    return tspContainer;
}

void tspContainerDestroy(tspContainer_t* tspContainer) {
    if (tspContainer->next != NULL)
        tspContainerDestroy(tspContainer->next);
    free(tspContainer);
}

static tspContainer_t* _tspContainerExpand(tspContainer_t* oldTspContainer) {
    tspContainer_t* newTspContainer = tspContainerCreate();
    newTspContainer->next = oldTspContainer;
    return newTspContainer;
}

tspNode_t* tspContainerFetchNode(tspContainer_t** tspContainerRef, const tspNode_t* parent, double cost, double lb, int length, int currentCity) {
    tspContainer_t* tspContainer = *tspContainerRef;
    if (tspContainer->curr == TSP_CONTAINER_SIZE) {
        tspContainer = _tspContainerExpand(tspContainer);
        *tspContainerRef = tspContainer;
    }

    tspNode_t* tspNode = &(tspContainer->nodes[tspContainer->curr++]);
    tspNode->parent = parent;
    tspNode->cost = cost;
    tspNode->lb = lb;
    tspNode->length = length;
    tspNode->currentCity = currentCity;
    return tspNode;
}