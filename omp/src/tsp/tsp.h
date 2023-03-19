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
    tspNode_t* solution;
    double bestTourCost;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads, double bestTourCost);
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
void tspSolve(tsp_t* tsp);

#endif // __TSP_TSP_H__