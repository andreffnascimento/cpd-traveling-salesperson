#ifndef __TSP__TSP_NODE_H__
#define __TSP__TSP_NODE_H__

#include "include.h"

typedef struct tspNode {
    double cost;
    double lb;
    size_t visited;
    size_t length;
    size_t* tour;
} tspNode_t;

tspNode_t* tspNodeCreate(double cost, double lb, size_t length, size_t currentCity);
tspNode_t* tspNodeExtend(const tspNode_t* parent, double cost, double lb, size_t currentCity);
void tspNodeCopyTour(const tspNode_t* tspNode, size_t* container);
void tspNodeDestroy(tspNode_t* node);
void tspNodePrint(const tspNode_t* node);

inline size_t tspNodeCurrentCity(const tspNode_t* node) { return node->tour[node->length - 1]; }

#endif // __TSP__TSP_NODE_H__
