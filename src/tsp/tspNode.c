#include "tspNode.h"

tspNode_t* tspNodeCreate(tspNode_t* parent, double cost, double lb, int length, int currentCity) {
    tspNode_t* node = (tspNode_t*)malloc(sizeof(tspNode_t));
    node->cost = cost;
    node->lb = lb;
    node->length = length;
    node->tour = (int*)malloc(sizeof(int) * node->length);
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
    printf("TSPNode{ currentCity = %d, cost = %f, lb = %f }\n - tour: ", tspNodeCurrentCity(node), node->cost, node->lb);
    for (size_t i = 0; i < node->length; i++)
        printf("%d > ", node->tour[i]);
    printf("\n");
}
