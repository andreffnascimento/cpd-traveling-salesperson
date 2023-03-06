#include "tsp.h"
#include "utils/queue.h"
#include <math.h>

tsp_t tspCreate(size_t nCities, size_t nRoads) {
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;

    // Initialization of the Roads to Null
    tsp.roadCosts = (double**)malloc(tsp.nRoads * sizeof(double*));
    for (size_t i = 0; i < tsp.nCities; i++) {
        tsp.roadCosts[i] = (double*)malloc(tsp.nRoads * sizeof(double));
        for (size_t j = 0; j < tsp.nCities; j++)
            tsp.roadCosts[i][j] = NONEXISTENT_ROAD_VALUE;
    }

    tsp.solution.hasSolution = false;
    tsp.solution.cost = INFINITY;
    tsp.solution.bestTour = NULL;
    tsp.solution.tspContainer = NULL;
    return tsp;
}

void tspDestroy(tsp_t* tsp) {
    tspContainerDestroy(tsp->solution.tspContainer);
    for (size_t i = 0; i < tsp->nCities; i++)
        free(tsp->roadCosts[i]);
    free(tsp->roadCosts);
    tsp->solution.bestTour = NULL;
}

void tspPrint(const tsp_t* tsp) {
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (size_t i = 0; i < tsp->nCities; i++)
        for (size_t j = 0; j < tsp->nCities; j++)
            printf(" - Road { %ld <-> %ld (cost = %f) }\n", i, j, tsp->roadCosts[i][j]);
}

static int _tspNodeCompFun(void* tspNode1, void* tspNode2) {
    return (((tspNode_t*)tspNode1)->lb <= ((tspNode_t*)tspNode2)->lb) ? 0 : 1;
}

static bool _isNeighbour(const tsp_t* tsp, int cityA, int cityB) {
    return (cityA != cityB) && (tsp->roadCosts[cityA][cityB] != NONEXISTENT_ROAD_VALUE);
}

static double _calculateInitialLb(const tsp_t* tsp) {
    double sum = 0.0;
    for (size_t i = 0; i < tsp->nCities; i++) {
        double min1 = INFINITY, min2 = INFINITY;
        for (size_t j = 0; j < tsp->nCities; j++) {
            double temp = tsp->roadCosts[i][j];
            if (_isNeighbour(tsp, i, j)) {
                if (temp < min1) {
                    min2 = min1;
                    min1 = temp;
                } else if (temp < min2)
                    min2 = temp;
            }
        }
        sum += min1 + min2;
    }
    return sum / 2;
}

static double _calculateLb(const tsp_t* tsp, const tspNode_t* node, int nextCity) {
    double min1From = INFINITY, min2From = INFINITY;
    double min1To = INFINITY, min2To = INFINITY;

    for (size_t i = 0; i < tsp->nCities; i++) {
        if (_isNeighbour(tsp, i, node->currentCity)) {
            double costFrom = tsp->roadCosts[i][node->currentCity];
            if (costFrom < min1From) {
                min2From = min1From;
                min1From = costFrom;
            } else if (costFrom < min2From) {
                min2From = tsp->roadCosts[i][node->currentCity];
            }
        }
        if (_isNeighbour(tsp, i, nextCity)) {
            double costTo = tsp->roadCosts[i][nextCity];
            if (costTo < min1To) {
                min2To = min1To;
                min1To = costTo;
            } else if (costTo < min2To) {
                min2To = costTo;
            }
        }
    }

    double costFromTo = tsp->roadCosts[node->currentCity][nextCity];
    double costFrom = (costFromTo >= min2From) ? min2From : min1From;
    double costTo = (costFromTo >= min2To) ? min2To : min1To;
    return node->lb + costFromTo - (costFrom + costTo) / 2;
}

static bool _isCityInTour(const tspNode_t* node, int cityNumber) {
    if (node == NULL)
        return false;
    else if (node->currentCity == cityNumber)
        return true;
    return _isCityInTour(node->parent, cityNumber);
}

const tspSolution_t* tspSolve(tsp_t* tsp, int maxValue) {
    tsp->solution.tspContainer = tspContainerCreate();
    tspNode_t* startNode = tspContainerFetchNode(&tsp->solution.tspContainer, NULL, 0, _calculateInitialLb(tsp), 1, 0);
    priorityQueue_t queue = queueCreate(&_tspNodeCompFun);
    queuePush(&queue, startNode);

    while (true) {
        tspNode_t* node = queuePop(&queue);
        if (node == NULL || node->lb >= maxValue) {
            queueDestroy(&queue);
            return &tsp->solution;
        }

        if (node->length == tsp->nCities && _isNeighbour(tsp, node->currentCity, 0)) { // we already visited all the cities
            double finalCost = node->cost + tsp->roadCosts[node->currentCity][0];
            if ((finalCost < maxValue) && (finalCost < tsp->solution.cost)) {
                tspNode_t* finalNode = tspContainerFetchNode(&tsp->solution.tspContainer, node, finalCost, _calculateLb(tsp, node, 0), node->length, 0);
                tsp->solution.hasSolution = true;
                tsp->solution.bestTour = finalNode;
                tsp->solution.cost = finalNode->cost;
            }

        } else {
            for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
                if (_isNeighbour(tsp, node->currentCity, cityNumber) && !_isCityInTour(node, cityNumber)) {
                    double lb = _calculateLb(tsp, node, cityNumber);
                    if (lb > maxValue)
                        continue;
                    
                    double cost = node->cost + tsp->roadCosts[node->currentCity][cityNumber];
                    tspNode_t* nextNode = tspContainerFetchNode(&tsp->solution.tspContainer, node, cost, lb, node->length + 1, cityNumber);
                    queuePush(&queue, nextNode);
                }
            }
        }
        //Need to delete node
    }
}
