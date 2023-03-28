#ifndef __TSP__TSP_H__
#define __TSP__TSP_H__

#include "include.h"
#include "tspNode.h"

#define NONEXISTENT_ROAD_VALUE -1
#define MAX_CITIES 64

typedef struct {
    size_t nCities;
    size_t nRoads;
    double** roadCosts;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads);
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);

#endif // __TSP__TSP_H__
