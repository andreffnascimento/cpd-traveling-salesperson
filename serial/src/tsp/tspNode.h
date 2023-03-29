#ifndef __TSP__TSP_NODE_H__
#define __TSP__TSP_NODE_H__

#include "include.h"

typedef struct tspNode {
    double cost;
    double lb;
    double priority;
    int length;
    char tour[MAX_CITIES];
    unsigned long long visited;
} tspNode_t;

tspNode_t* tspNodeInit(tspNode_t* node, double cost, double lb, int length, int currentCity);
tspNode_t* tspNodeExtInit(tspNode_t* node, const tspNode_t* parent, double cost, double lb, int currentCity);

tspNode_t* tspNodeCreate(double cost, double lb, int length, int currentCity);
tspNode_t* tspNodeExtend(const tspNode_t* parent, double cost, double lb, int currentCity);

void tspNodeCopyTour(const tspNode_t* tspNode, char* container);
void tspNodeDestroy(tspNode_t* node);
void tspNodePrint(const tspNode_t* node);

inline int tspNodeCurrentCity(const tspNode_t* node) { return node->tour[node->length - 1]; }

#endif // __TSP__TSP_NODE_H__
