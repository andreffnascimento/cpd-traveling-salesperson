#include "tspNode.h"

tspNode_t* tspNodeCreate(double cost, double lb, int length, int currentCity) {
    tspNode_t* node = (tspNode_t*)malloc(sizeof(tspNode_t));
    node->cost = cost;
    node->lb = lb;
    node->priority = lb * MAX_CITIES + currentCity;
    node->length = length;
    node->tour[node->length - 1] = currentCity;
    node->visited = 0x00000001 << currentCity;
    return node;
}

tspNode_t* tspNodeExtend(const tspNode_t* parent, double cost, double lb, int currentCity) {
    tspNode_t* node = tspNodeCreate(cost, lb, parent->length + 1, currentCity);
    node->visited |= parent->visited;
    tspNodeCopyTour(parent, node->tour);
    return node;
}

void tspNodeDestroy(tspNode_t* node) {
    free(node);
    node = NULL;
}

void tspNodeCopyTour(const tspNode_t* tspNode, char* container) {
    for (int i = 0; i < tspNode->length; i++)
        container[i] = tspNode->tour[i];
}

void tspNodePrint(const tspNode_t* node) {
    printf("TSPNode{ currentCity = %d, cost = %f, lb = %f, length = %d }\n - tour: ", tspNodeCurrentCity(node),
           node->cost, node->lb, node->length);
    for (int i = 0; i < node->length; i++)
        printf("%d > ", node->tour[i]);
    printf("\n");
}
