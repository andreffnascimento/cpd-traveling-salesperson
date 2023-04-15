#ifndef __TSP__TSP_H__
#define __TSP__TSP_H__

#include "include.h"

#define MAX_CITIES 64
#define NONEXISTENT_ROAD_VALUE -1

#define TSP_TOTAL_MIN_COSTS 2
#define TSP_MIN_COSTS_1 0
#define TSP_MIN_COSTS_2 1

typedef struct {
    int nCities;
    double roadCosts[MAX_CITIES][MAX_CITIES];
    double minCosts[MAX_CITIES][TSP_TOTAL_MIN_COSTS];
} tsp_t;

tsp_t* tspCreate();
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
void tspInitializeMinCosts(tsp_t* tsp);

inline bool tspIsNeighbour(const tsp_t* tsp, int city1, int city2) {
    return tsp->roadCosts[city1][city2] != NONEXISTENT_ROAD_VALUE;
}

inline double tspMinCost(const tsp_t* tsp, int city, int mod) { return tsp->minCosts[city][mod]; }

#endif // __TSP__TSP_H__
