#include "tsp.h"
#include "utils/queue.h"
#include <math.h>

static int _tspNodeCompFun(void* tspNode1, void* tspNode2) {
    tspNode_t* n1 = (tspNode_t*)tspNode1;
    tspNode_t* n2 = (tspNode_t*)tspNode2;
    return (n2->lb < n1->lb || ((n2->lb == n1->lb && tspNodeCurrentCity(n2) < tspNodeCurrentCity(n1)))) ? 1 : 0;
}

static void _queueDeleteFun(void* node) {
    tspNode_t* n = (tspNode_t*)node;
    free(n);
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

    return tsp;
}

void tspDestroy(tsp_t* tsp) {
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

static bool _isNeighbour(const tsp_t* tsp, size_t cityA, size_t cityB) {
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

static double _calculateLb(const tsp_t* tsp, const tspNode_t* node, size_t nextCity) {
    double min1From = INFINITY, min2From = INFINITY;
    double min1To = INFINITY, min2To = INFINITY;
    size_t nodeCurrentCity = tspNodeCurrentCity(node);

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

static bool _isCityInTour(const tspNode_t* node, size_t cityNumber) {
    for (size_t i = 0; i < node->length; i++)
        if (node->tour[i] == cityNumber)
            return true;
    return false;
}

static tspNode_t* _getNextNode(priorityQueue_t* queue, double maxTourCost) {
    tspNode_t* node;

#pragma omp critical(queue)
    node = queuePop(queue);

    if (node != NULL && node->lb >= maxTourCost) {
        tspNodeDestroy(node);
        return NULL;
    }
    return node;
}

static void _updateBestTour(const tsp_t* tsp, tspNode_t** solution, const tspNode_t* node) {
    size_t nodeCurrentCity = tspNodeCurrentCity(node);
    size_t solutionCurrentCity = tspNodeCurrentCity(*solution);
    double cost = node->cost + tsp->roadCosts[nodeCurrentCity][0];
    tspNode_t* oldSolution = *solution;
    if (cost < oldSolution->cost || (cost == oldSolution->cost && nodeCurrentCity < solutionCurrentCity)) {
        tspNode_t* newSolution = tspNodeCreate(cost, _calculateLb(tsp, node, 0), node->length + 1, nodeCurrentCity);
        tspNodeCopyTour(node, newSolution);

#pragma omp critical(solution)
        {
            *solution = newSolution;
            tspNodeDestroy(oldSolution);
        }
    }
}

static void _visitNeighbors(const tsp_t* tsp, priorityQueue_t* queue, const tspNode_t* solution,
                            const tspNode_t* node) {
    size_t nodeCurrentCity = tspNodeCurrentCity(node);
    for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (_isNeighbour(tsp, nodeCurrentCity, cityNumber) && !_isCityInTour(node, cityNumber)) {
            double lb = _calculateLb(tsp, node, cityNumber);
            if (lb > solution->cost)
                continue;
            double cost = node->cost + tsp->roadCosts[nodeCurrentCity][cityNumber];
            tspNode_t* nextNode = tspNodeCreate(cost, lb, node->length + 1, cityNumber);
            tspNodeCopyTour(node, nextNode);

#pragma omp critical(queue)
            queuePush(queue, nextNode);
        }
    }
}

static void _processNode(const tsp_t* tsp, priorityQueue_t* queue, tspNode_t** solution, tspNode_t* node) {
    DEBUG(tspNodePrint(node));
    size_t nodeCurrentCity = tspNodeCurrentCity(node);
    if ((node->length == tsp->nCities) && _isNeighbour(tsp, nodeCurrentCity, 0))
        _updateBestTour(tsp, solution, node);
    else
        _visitNeighbors(tsp, queue, *solution, node);
    tspNodeDestroy(node);
}

tspNode_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    tspNode_t* solution = tspNodeCreate(maxTourCost, -1, 1, 0);
    priorityQueue_t queue = queueCreate(_tspNodeCompFun);
    tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(tsp), 1, 0);
    queuePush(&queue, startNode);

#pragma omp parallel
    {
        while (true) {
            tspNode_t* node = _getNextNode(&queue, maxTourCost);
            if (node == NULL)
                break;
            _processNode(tsp, &queue, &solution, node);
        }
    }

    queueDelete(&queue, _queueDeleteFun);
    return solution;
}
