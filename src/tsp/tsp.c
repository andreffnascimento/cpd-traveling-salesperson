#include "tsp.h"

tsp_t tspCreate(size_t nCities, size_t nRoads)
{
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;
    tsp.roads = malloc(tsp.nRoads * sizeof(tspRoad_t));
    tsp.solution.hasSolution = false;
    tsp.solution.cost = -1.0f;
    // ISSUE: excessive amount of allocation of memory with the possible result
    tsp.solution.cities = malloc(tsp.nCities * sizeof(int));
    return tsp;
}

void tspDelete(tsp_t *tsp)
{
    free(tsp->solution.cities);
    free(tsp->roads);
    tsp->solution.cities = NULL;
    tsp->roads = NULL;
}

void tspPrint(const tsp_t *tsp)
{
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (int i = 0; i < tsp->nRoads; i++)
    {
        tspRoad_t *road = &tsp->roads[i];
        printf(" - Road[%d]{ %d <-> %d (cost = %d) }\n", i, road->cityA, road->cityB, road->cost);
    }
}

tspNode_t *tspCreateNode(int *tour, double cost, double lb, int length, int currentCity) {
    tspNode_t *node = malloc(sizeof(tspNode_t));
    node->cost = cost;
    node->lb = lb;
    node->length = length;
    node->currentCity = currentCity;
    return node;

}

void tspSolve(const tsp_t *tsp, double max_value) {
    tspNode_t *node = tspCreateNode();

    

}
