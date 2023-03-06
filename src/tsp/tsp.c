#include "tsp.h"
#include <math.h>
#include "utils/queue.h"

int emptyFunc(void *a, void *b) {return 0;}

int compFun(tspNode_t *n1, tspNode_t *n2) {
    if (n1->lb <= n2->lb) return 0;
    return 1;
}

tsp_t tspCreate(size_t nCities, size_t nRoads) {
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;

    // Initialization of the Roads to Null
    tsp.roadCosts = (double **)malloc(tsp.nRoads * sizeof(double *));
    for (size_t i = 0; i < tsp.nCities; i++) {
        tsp.roadCosts[i] = (double *)malloc(tsp.nRoads * sizeof(double));
        for (size_t j = 0; j < tsp.nCities; j++)
        {
            tsp.roadCosts[i][j] = NONEXISTENT_ROAD_VALUE;
        }
    }

    tsp.trashQueue = queueCreate(&emptyFunc);
    tsp.queue = queueCreate(&compFun);
    tsp.solution.hasSolution = false;
    tsp.solution.cost = INFINITY;
    tsp.solution.bestTour = NULL;
    return tsp;
}

void tspDestroy(tsp_t *tsp) {
    while(true) {
        tspNode_t *node = queuePop(&tsp->trashQueue);     
        if (node == NULL) break;   
        tspDestroyNode(node);
    }
    for (size_t i = 0; i < tsp->nCities; i++) free(tsp->roadCosts[i]);
    free(tsp->roadCosts);
    queueDelete(&tsp->queue);
    queueDelete(&tsp->trashQueue);
    tsp->solution.bestTour = NULL;
    tsp->roadCosts = NULL;
}

void tspPrint(const tsp_t* tsp) {
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (size_t i = 0; i < tsp->nCities; i++)
        for (size_t j = 0; j < tsp->nCities; j++)
            printf(" - Road { %ld <-> %ld (cost = %f) }\n", i, j, tsp->roadCosts[i][j]);
}

tspNode_t *tspCreateNode(tspNode_t *parent, double cost, double lb, int length, int currentCity) {

    tspNode_t *node = malloc(sizeof(tspNode_t));
    node->parent = parent; // should be a queue?
    node->cost = cost;
    node->lb = lb;
    node->length = length;
    node->currentCity = currentCity;
    return node;
}

void tspDestroyNode(tspNode_t *node) {
    free(node);
    node=NULL;
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

void tspSolve(tsp_t *tsp, int maxValue) {
    tspNode_t *startNode = tspCreateNode(NULL, 0, _calculateInitialLb(tsp), 1, 0);
    queuePush(&tsp->trashQueue,startNode);
    queuePush(&tsp->queue, startNode);

    tspNode_t *node;
    double finalCost;
    while (true) {
        node = queuePop(&tsp->queue);
        if (node == NULL) return;

        if (node->lb >= maxValue) {
            return;
        }

        if (node->length == tsp->nCities && _isNeighbour(tsp, node->currentCity, 0)) {
            finalCost = node->cost + tsp->roadCosts[node->currentCity][0];
            if ((finalCost < maxValue) && (finalCost < tsp->solution.cost)) {
                tspNode_t *finalNode = tspCreateNode(node, finalCost, _calculateLb(tsp, node, 0), node->length, 0);
                tsp->solution.hasSolution = true;
                tsp->solution.bestTour = finalNode;
                tsp->solution.cost = finalNode->cost;
                queuePush(&tsp->trashQueue,finalNode);
            }
        }
        else {
            double lb;
            for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {   
                if (_isNeighbour(tsp, node->currentCity, cityNumber) && !_isCityInTour(node, cityNumber)) {
                    lb = _calculateLb(tsp, node, cityNumber);
                    
                    if (lb > maxValue) continue;

                    tspNode_t *nextNode = tspCreateNode(node, node->cost + tsp->roadCosts[node->currentCity][cityNumber], lb, node->length + 1, cityNumber);
                    queuePush(&tsp->queue, nextNode);
                    queuePush(&tsp->trashQueue,nextNode);
                }
            }
            
        }
    }
}
