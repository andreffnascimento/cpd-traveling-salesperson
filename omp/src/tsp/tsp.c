#include "tsp.h"
#include <math.h>

void _init_road_costs(tsp_t* tsp) {
    for (int i = 0; i < tsp->nCities; i++) {
        tsp->roadCosts[i] = (double*)malloc(tsp->nRoads * sizeof(double));
        tsp->minCosts[i * 2] = tsp->minCosts[i * 2 + 1] = INFINITY;
        for (int j = 0; j < tsp->nCities; j++)
            tsp->roadCosts[i][j] = NONEXISTENT_ROAD_VALUE;
    }
}

tsp_t tspCreate(int nCities, int nRoads) {
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;
    tsp.roadCosts = (double**)malloc(tsp.nRoads * sizeof(double*));
    tsp.minCosts = (double*)malloc(tsp.nRoads * sizeof(double) * TSP_TOTAL_MIN_COSTS);
    _init_road_costs(&tsp);
    return tsp;
}

void tspDestroy(tsp_t* tsp) {
    for (int i = 0; i < tsp->nCities; i++)
        free(tsp->roadCosts[i]);
    free(tsp->roadCosts);
    free(tsp->minCosts);
}

void tspPrint(const tsp_t* tsp) {
    printf("TSP{ nCities = %d, nRoads = %d }\n", tsp->nCities, tsp->nRoads);
    for (int i = 0; i < tsp->nCities; i++) {
        printf("- Road %d (min1 = %f ; min2 = %f)\n", i, tspMinCost(tsp, i, TSP_MIN_COSTS_1),
               tspMinCost(tsp, i, TSP_MIN_COSTS_2));
        for (int j = 0; j < tsp->nCities; j++) {
            if (tsp->roadCosts[i][j] != NONEXISTENT_ROAD_VALUE)
                printf("\t%d <-> %d (cost = %f)\n", i, j, tsp->roadCosts[i][j]);
        }
    }
}

void tspInitializeMinCosts(tsp_t* tsp) {
    for (int i = 0; i < tsp->nCities; i++) {
        double min1 = INFINITY, min2 = INFINITY;
        for (int j = 0; j < tsp->nCities; j++) {
            if (tspIsNeighbour(tsp, i, j)) {
                double costIn = tsp->roadCosts[j][i];
                if (costIn < min1) {
                    min2 = min1;
                    min1 = costIn;
                } else if (costIn < min2) {
                    min2 = costIn;
                }
            }
        }

        tsp->minCosts[i * TSP_TOTAL_MIN_COSTS + TSP_MIN_COSTS_1] = min1;
        tsp->minCosts[i * TSP_TOTAL_MIN_COSTS + TSP_MIN_COSTS_2] = min2;
    }
}