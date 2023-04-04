#ifndef __TSP__TSP_NODE_H__
#define __TSP__TSP_NODE_H__

#include "include.h"

typedef struct Node {
    double cost;
    double lb;
    double priority;
    int length;
    char tour[MAX_CITIES];
    unsigned long long visited;
} Node_t;

Node_t* nodeInit(Node_t* node, double cost, double lb, int length, int currentCity);
Node_t* nodeExtInit(Node_t* node, const Node_t* parent, double cost, double lb, int currentCity);

Node_t* nodeCreate(double cost, double lb, int length, int currentCity);
Node_t* nodeExtend(const Node_t* parent, double cost, double lb, int currentCity);

void nodeCopyTour(const Node_t* node, char* container);
void nodeDestroy(Node_t* node);
void nodePrint(const Node_t* node);

inline int nodeCurrentCity(const Node_t* node) { return node->tour[node->length - 1]; }

#endif // __TSP__TSP_NODE_H__
