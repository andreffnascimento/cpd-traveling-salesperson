#ifndef __TSP_TSP_H__
#define __TSP_TSP_H__

#include "include.h"

typedef struct {
    int cityA;
    int cityB;
    int cost;
} tspRoad_t;

typedef struct {
    bool hasSolution;
    float cost;
    int* cities;
} tspSolution_t;

typedef struct {
    size_t nCities;
    size_t nRoads;
    tspRoad_t* roads;
    tspSolution_t solution;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads);

void tspDelete(tsp_t* tsp);

void tspPrint(const tsp_t* tsp);

void tspSolve(const tsp_t* tsp);

#endif // __TSP_TSP_H__