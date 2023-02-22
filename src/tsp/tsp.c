#include "tsp.h"

tsp_t tspCreate(size_t nCities, size_t nRoads) {
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;
    tsp.roads = malloc(tsp.nRoads * sizeof(tspRoad_t));
    tsp.solution.hasSolution = false;
    tsp.solution.cost = -1.0f;
    tsp.solution.cities = malloc(tsp.nCities * sizeof(int));
    return tsp;
}

void tspDelete(tsp_t* tsp) {
    free(tsp->solution.cities);
    free(tsp->roads);
    tsp->solution.cities = NULL;
    tsp->roads = NULL;
}

void tspPrint(const tsp_t* tsp) {
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (int i = 0; i < tsp->nRoads; i++) {
        tspRoad_t* road = &tsp->roads[i];
        printf(" - Road[%d]{ %d <-> %d (cost = %d) }\n", i, road->cityA, road->cityB, road->cost);
    }
}

void tspSolve(const tsp_t* tsp) {
}
