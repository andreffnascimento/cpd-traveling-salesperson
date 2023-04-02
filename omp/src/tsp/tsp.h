#ifndef __TSP__TSP_H__
#define __TSP__TSP_H__

#include "include.h"

#define TSP_TOTAL_MIN_COSTS 2
#define TSP_MIN_COSTS_1 0
#define TSP_MIN_COSTS_2 1

typedef struct {
    int nCities;
    int nRoads;
    double** roadCosts;
    double* minCosts;
} tsp_t;

tsp_t tspCreate(int nCities, int nRoads);
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
void tspInitializeMinCosts(tsp_t* tsp);

inline bool tspIsNeighbour(const tsp_t* tsp, int city1, int city2) {
    return tsp->roadCosts[city1][city2] != NONEXISTENT_ROAD_VALUE;
}

inline double tspMinCost(const tsp_t* tsp, int city, int mod) {
    return tsp->minCosts[city * TSP_TOTAL_MIN_COSTS + mod];
}

#endif // __TSP__TSP_H__
