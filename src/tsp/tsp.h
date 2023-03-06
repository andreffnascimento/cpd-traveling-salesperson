#ifndef __TSP_TSP_H__
#define __TSP_TSP_H__

#include "include.h"
#include "tspContainer.h"

#define NONEXISTENT_ROAD_VALUE -1

/*----------------------------------------------------------
_____________________________tsp____________________________
----------------------------------------------------------*/

typedef struct {
    bool hasSolution;
    double cost;
    tspNode_t* bestTour;
    tspContainer_t* tspContainer;
} tspSolution_t;

typedef struct {
    size_t nCities;
    size_t nRoads;
    double** roadCosts;
    tspSolution_t solution;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads);
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
const tspSolution_t* tspSolve(tsp_t* tsp, int maxValue);

#endif // __TSP_TSP_H__