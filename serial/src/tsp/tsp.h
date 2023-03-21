#ifndef __TSP_TSP_H__
#define __TSP_TSP_H__

#include "include.h"
#include "tspNode.h"

#define NONEXISTENT_ROAD_VALUE -1

/***********************************************************
-------------------------- TSP -----------------------------
***********************************************************/

typedef struct {
    size_t nCities;
    size_t nRoads;
    double** roadCosts;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads);
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
tspNode_t* tspSolve(const tsp_t* tsp, double maxTourCost);

#endif // __TSP_TSP_H__
