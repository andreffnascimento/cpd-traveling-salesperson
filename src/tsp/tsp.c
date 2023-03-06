#include "tsp.h"
#include <math.h>
#include "utils/queue.h"

static int _emptyFunc(void *a, void *b) {return 0;}

static int _tspNodeCompFun(void* tspNode1, void* tspNode2) {
    return (((tspNode_t*)tspNode1)->lb <= ((tspNode_t*)tspNode2)->lb) ? 0 : 1;
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

    tsp.trashQueue = queueCreate(&_emptyFunc);
    tsp.queue = queueCreate(&_tspNodeCompFun);
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
    while(true) {
        tspNode_t *node = queuePop(&tsp->queue);     
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

tspSmallNode_t* tspCreateSmallNode(tspSmallNode_t* parent, int currentCity) {
    tspSmallNode_t *smallNode = (tspSmallNode_t *)malloc(sizeof(tspSmallNode_t));
    smallNode->parent = parent;
    smallNode->currentCity = currentCity;
    return smallNode;
}
void tspDestroySmallNode(tspSmallNode_t* node) {
    free(node);
}

void tspPrint(const tsp_t* tsp) {
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (size_t i = 0; i < tsp->nCities; i++)
        for (size_t j = 0; j < tsp->nCities; j++)
            printf(" - Road { %ld <-> %ld (cost = %f) }\n", i, j, tsp->roadCosts[i][j]);
}

tspNode_t *tspCreateNode(tspSmallNode_t *parent, double cost, double lb, int length, int currentCity) {

    tspNode_t *node = malloc(sizeof(tspNode_t));
    node->parent = parent;
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

static bool _isCityInTour(const tspSmallNode_t* node, int cityNumber) {
    if (node == NULL)
        return false;
    else if (node->currentCity == cityNumber)
        return true;
    return _isCityInTour(node->parent, cityNumber);
}

void tspSolve(tsp_t *tsp, int maxValue) {
    tspNode_t *startNode,*node;
    double finalCost;

    tspSmallNode_t *startSmallNode = tspCreateSmallNode(NULL, 0);
    startNode = tspCreateNode(startSmallNode, 0, _calculateInitialLb(tsp), 1, 0);

    queuePush(&tsp->queue, startNode);
    queuePush(&tsp->trashQueue, startSmallNode);
    
    while (true) {
        node = queuePop(&tsp->queue);
        if (node == NULL) return;

        if (node->lb >= maxValue) {
            tspDestroyNode(node);
            return;
        }

        if (node->length == tsp->nCities && _isNeighbour(tsp, node->currentCity, 0)) {
            finalCost = node->cost + tsp->roadCosts[node->currentCity][0];
            if ((finalCost < maxValue) && (finalCost < tsp->solution.cost)) {

                tspSmallNode_t *smallFinalNode = tspCreateSmallNode(node->parent, 0);
                tspNode_t *finalNode = tspCreateNode(smallFinalNode, finalCost, _calculateLb(tsp, node, 0), node->length, 0);
                
                tsp->solution.hasSolution = true;
                tsp->solution.bestTour = smallFinalNode;
                tsp->solution.cost = finalNode->cost;
                
                tspDestroyNode(finalNode);
                queuePush(&tsp->trashQueue, smallFinalNode);
            }
        }
        else {
            double lb;
            for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {   
                if (_isNeighbour(tsp, node->currentCity, cityNumber) && !_isCityInTour(node->parent, cityNumber)) {
                    lb = _calculateLb(tsp, node, cityNumber);
                    
                    if (lb > maxValue) continue;

                    tspSmallNode_t *nextSmallNode = tspCreateSmallNode(node->parent, cityNumber);
                    tspNode_t *nextNode = tspCreateNode(nextSmallNode, node->cost + tsp->roadCosts[node->currentCity][cityNumber], lb, node->length + 1, cityNumber);
                    queuePush(&tsp->queue, nextNode);
                    queuePush(&tsp->trashQueue, nextSmallNode);

                }
            }
        }
        tspDestroyNode(node);
    }
}
