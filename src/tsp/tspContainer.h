#ifndef __TSP_TSP_CONTAINER_H__
#define __TSP_TSP_CONTAINER_H__

#include "include.h"

#define TSP_CONTAINER_SIZE 1024

/*----------------------------------------------------------
__________________________tsp Node__________________________
----------------------------------------------------------*/

typedef struct tspNode {
    const struct tspNode* parent;
    double cost;
    double lb;
    int length;
    int currentCity;
} tspNode_t;

/*----------------------------------------------------------
_______________________tsp Container________________________
----------------------------------------------------------*/

typedef struct tspContainer {
    tspNode_t nodes[TSP_CONTAINER_SIZE];
    size_t curr;
    struct tspContainer* next;
} tspContainer_t;

tspContainer_t* tspContainerCreate();
void tspContainerDestroy(tspContainer_t* tspContainer);
tspNode_t* tspContainerFetchNode(tspContainer_t** tspContainerRef, const tspNode_t* parent, double cost, double lb, int length, int currentCity);

#endif