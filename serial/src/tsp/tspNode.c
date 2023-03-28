#include "tspNode.h"

tspNode_t* tspNodeCreate(double cost, double lb, size_t length, size_t currentCity) {
    tspNode_t* node = (tspNode_t*)malloc(sizeof(tspNode_t));
    node->cost = cost;
    node->lb = lb;
    node->visited = 0x00000001 << currentCity;
    node->length = length;
    node->tour = (size_t*)malloc(sizeof(size_t) * node->length);
    node->tour[node->length - 1] = currentCity;
    return node;
}

tspNode_t* tspNodeExtend(const tspNode_t* parent, double cost, double lb, size_t currentCity) {
    tspNode_t* node = tspNodeCreate(cost, lb, parent->length + 1, currentCity);
    node->visited |= parent->visited;
    tspNodeCopyTour(parent, node->tour);
    return node;
}

void tspNodeCopyTour(const tspNode_t* tspNode, size_t* container) {
    for (size_t i = 0; i < tspNode->length; i++)
        container[i] = tspNode->tour[i];
}

void tspNodeDestroy(tspNode_t* node) {
    free(node->tour);
    free(node);
    node = NULL;
}

void tspNodePrint(const tspNode_t* node) {
    printf("TSPNode{ currentCity = %ld, cost = %f, lb = %f, length = %ld }\n - tour: ", tspNodeCurrentCity(node),
           node->cost, node->lb, node->length);
    for (size_t i = 0; i < node->length; i++)
        printf("%ld > ", node->tour[i]);
    printf("\n");
}
