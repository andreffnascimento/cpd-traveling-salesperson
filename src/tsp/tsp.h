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
    double cost;
    int* cities;
} tspSolution_t;

typedef struct {
    size_t nCities;
    size_t nRoads;
    tspRoad_t* roads;
    tspSolution_t solution;
} tsp_t;

typedef struct {
    int* tour;
    double cost;
    double lb;
    int length;
    int* currentCity;

} tspNode_t;

tspNode_t* tspCreateNode(int* tour, double cost, double lb, int length, int currentCity);

void tspDeleteNode(tspNode_t *node);

tsp_t tspCreate(size_t nCities, size_t nRoads);

void tspDelete(tsp_t* tsp);

void tspPrint(const tsp_t* tsp);

void tspSolve(const tsp_t* tsp);

#endif // __TSP_TSP_H__