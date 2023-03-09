#include "tsp.h"
#include "utils/queue.h"
#include <math.h>

static int _tspNodeCompFun(void* tspNode1, void* tspNode2) {
    tspNode_t* n1 = (tspNode_t*)tspNode1;
    tspNode_t* n2 = (tspNode_t*)tspNode2;
    return (n2->lb < n1->lb || ((n2->lb == n1->lb && tspNodeCurrentCity(n2) < tspNodeCurrentCity(n1)))) ? 1 : 0;
}

tsp_t tspCreate(size_t nCities, size_t nRoads) {
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;

    // Initialization of th\e Roads to Null
    tsp.roadCosts = (double**)malloc(tsp.nRoads * sizeof(double*));
    for (size_t i = 0; i < tsp.nCities; i++) {
        tsp.roadCosts[i] = (double*)malloc(tsp.nRoads * sizeof(double));
        for (size_t j = 0; j < tsp.nCities; j++)
            tsp.roadCosts[i][j] = NONEXISTENT_ROAD_VALUE;
    }

    tsp.queue = queueCreate(&_tspNodeCompFun);
    return tsp;
}

void tspDestroy(tsp_t* tsp) {
    tspNode_t* node;
    while ((node = queuePop(&tsp->queue)) != NULL)
        tspNodeDestroy(node);
    queueDelete(&tsp->queue);
    for (size_t i = 0; i < tsp->nCities; i++)
        free(tsp->roadCosts[i]);
    free(tsp->roadCosts);
    tsp->roadCosts = NULL;
}

void tspPrint(const tsp_t* tsp) {
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (size_t i = 0; i < tsp->nCities; i++)
        for (size_t j = 0; j < tsp->nCities; j++)
            printf(" - Road { %ld <-> %ld (cost = %f) }\n", i, j, tsp->roadCosts[i][j]);
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
    int nodeCurrentCity = tspNodeCurrentCity(node);

    for (size_t i = 0; i < tsp->nCities; i++) {
        if (_isNeighbour(tsp, i, nodeCurrentCity)) {
            double costFrom = tsp->roadCosts[i][nodeCurrentCity];
            if (costFrom < min1From) {
                min2From = min1From;
                min1From = costFrom;
            } else if (costFrom < min2From) {
                min2From = tsp->roadCosts[i][nodeCurrentCity];
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

    double costFromTo = tsp->roadCosts[nodeCurrentCity][nextCity];
    double costFrom = (costFromTo >= min2From) ? min2From : min1From;
    double costTo = (costFromTo >= min2To) ? min2To : min1To;
    return node->lb + costFromTo - (costFrom + costTo) / 2;
}

static bool _isCityInTour(const tspNode_t* node, int cityNumber) {
    for (size_t i = 0; i < node->length; i++)
        if (node->tour[i] == cityNumber)
            return true;
    return false;
}

tspNode_t* tspSolve(tsp_t* tsp, double maxTourCost) {
    tspNode_t* solution = NULL;
    tspNode_t* startNode = tspNodeCreate(NULL, 0, _calculateInitialLb(tsp), 1, 0);
    queuePush(&tsp->queue, startNode);

    while (true) {
        tspNode_t* node = queuePop(&tsp->queue);
        if (node == NULL)
            return solution;

        if (node->lb >= maxTourCost) {
            tspNodeDestroy(node);
            return solution;
        }

        DEBUG(tspNodePrint(node));
        int nodeCurrentCity = tspNodeCurrentCity(node);
        if ((node->length == tsp->nCities) && _isNeighbour(tsp, nodeCurrentCity, 0)) {
            double cost = node->cost + tsp->roadCosts[nodeCurrentCity][0];
            if (cost < maxTourCost) {
                if (solution != NULL)
                    tspNodeDestroy(solution);

                solution = tspNodeCreate(node, cost, _calculateLb(tsp, node, 0), node->length + 1, 0);
                tspNodeCopyTour(node, solution);
                DEBUG(tspNodePrint(solution));
                maxTourCost = cost;
            }
        } else {
            for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
                if (_isNeighbour(tsp, nodeCurrentCity, cityNumber) && !_isCityInTour(node, cityNumber)) {
                    double lb = _calculateLb(tsp, node, cityNumber);
                    if (lb > maxTourCost)
                        continue;
                    double cost = node->cost + tsp->roadCosts[nodeCurrentCity][cityNumber];
                    tspNode_t* nextNode = tspNodeCreate(node, cost, lb, node->length + 1, cityNumber);
                    tspNodeCopyTour(node, nextNode);
                    queuePush(&tsp->queue, nextNode);
                }
            }
        }
        tspNodeDestroy(node);
    }
}
