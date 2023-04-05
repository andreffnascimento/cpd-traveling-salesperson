#include "node.h"

Node_t* nodeInit(Node_t* node, double cost, double lb, int length, int currentCity) {
    //Initialize the Node with cost, lb, length & currentCity
    node->cost = cost;
    node->lb = lb;
    node->priority = lb * MAX_CITIES + currentCity;
    node->length = length;
    node->tour[node->length - 1] = currentCity;
    node->visited = 0x00000001 << currentCity;
    return node;
}

Node_t* nodeExtInit(Node_t* node, const Node_t* parent, double cost, double lb, int currentCity) {
    //
    nodeInit(node, cost, lb, parent->length + 1, currentCity);
    node->visited |= parent->visited; //or operator
    nodeCopyTour(parent, node->tour);
    return node;
}

Node_t* nodeCreate(double cost, double lb, int length, int currentCity) {
    Node_t* node = (Node_t*)malloc(sizeof(Node_t));
    node->cost = cost;
    node->lb = lb;
    node->priority = lb * MAX_CITIES + currentCity;
    node->length = length;
    node->tour[node->length - 1] = currentCity;
    node->visited = 0x00000001 << currentCity;
    return node;
}

Node_t* nodeExtend(const Node_t* parent, double cost, double lb, int currentCity) {
    Node_t* node = nodeCreate(cost, lb, parent->length + 1, currentCity);
    node->visited |= parent->visited;
    nodeCopyTour(parent, node->tour);
    return node;
}

void nodeCopyTour(const Node_t* node, char* tour) {
    for (int i = 0; i < node->length; i++)
        tour[i] = node->tour[i];
}

void nodeDestroy(Node_t* node) {
    free(node);
    node = NULL;
}

void nodePrint(const Node_t* node) {
    printf("TSPNode{ currentCity = %d, cost = %f, lb = %f, length = %d }\n - tour: ", nodeCurrentCity(node),
           node->cost, node->lb, node->length);
    for (int i = 0; i < node->length; i++)
        printf("%d > ", node->tour[i]);
    printf("\n");
}
