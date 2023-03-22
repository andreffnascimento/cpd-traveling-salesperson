#ifndef __TSP_TSP_NODE_H__
#define __TSP_TSP_NODE_H__

#include "include.h"

/***********************************************************
------------------------ TSP NODE --------------------------
***********************************************************/

typedef struct tspNode {
    double cost;
    double lb;
    size_t length;
    size_t* tour;
} tspNode_t;

tspNode_t* tspNodeCreate(double cost, double lb, size_t length, size_t currentCity);
void tspNodeCopyTour(const tspNode_t* parent, tspNode_t* child);
void tspNodeDestroy(tspNode_t* node);
void tspNodePrint(const tspNode_t* node);

inline size_t tspNodeCurrentCity(const tspNode_t* node) { return node->tour[node->length - 1]; }
inline size_t tspNodePrevCity(const tspNode_t* node, size_t prev) { return node->tour[node->length - prev - 1]; }

#endif // __TSP_TSP_NODE_H__
