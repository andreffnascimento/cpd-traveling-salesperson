#include "tspNode.h"

tspNode_t* tspNodeCreate(double cost, double lb, size_t length, size_t currentCity) {
    tspNode_t* node = (tspNode_t*)malloc(sizeof(tspNode_t));
    node->cost = cost;
    node->lb = lb;
    node->length = length;
    node->tour = (size_t*)malloc(sizeof(size_t) * node->length);
    node->tour[node->length - 1] = currentCity;
    return node;
}

void tspNodeCopyTour(const tspNode_t* parent, tspNode_t* child) {
    for (size_t i = 0; i < parent->length; i++)
        child->tour[i] = parent->tour[i];
}

void tspNodeDestroy(tspNode_t* node) {
    free(node->tour);
    free(node);
    node = NULL;
}

void tspNodePrint(const tspNode_t* node) {
    printf("TSPNode{ currentCity = %ld, cost = %f, lb = %f }\n - tour: ", tspNodeCurrentCity(node), node->cost,
           node->lb);
    for (size_t i = 0; i < node->length; i++)
        printf("%ld > ", node->tour[i]);
    printf("\n");
}
