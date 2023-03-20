#include "tsp.h"
#include "utils/queue.h"
#include <math.h>

void processNode(tsp_t* tsp);

static int _tspNodeCompFun(void* tspNode1, void* tspNode2) {
    tspNode_t* n1 = (tspNode_t*)tspNode1;
    tspNode_t* n2 = (tspNode_t*)tspNode2;
    return (n2->lb < n1->lb || ((n2->lb == n1->lb && tspNodeCurrentCity(n2) < tspNodeCurrentCity(n1)))) ? 1 : 0;
}

tsp_t tspCreate(size_t nCities, size_t nRoads, double bestTourCost) {
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;
    tsp.bestTourCost = bestTourCost;
    tsp.solution = NULL;
    tsp.queue = queueCreate(_tspNodeCompFun); //TODO: future initialize with N number of Cores, maybe static var

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
    
    tspNode_t* node;
    while ((node = queuePop(&tsp->queue)) != NULL)
        tspNodeDestroy(node);
    queueDelete(&tsp->queue);
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

bool verifyNode(tsp_t*tsp, tspNode_t* node) {

    if (!node) return false;
    else if (node->lb >= tsp->bestTourCost) {
        tspNodeDestroy(node);
        return false;
    }
    return true;

}

bool updateBestTour(tsp_t* tsp, tspNode_t* node, size_t nodeCurrentCity) {
    bool res = false;
    // #pragma omp single
    if ((node->length == tsp->nCities) && _isNeighbour(tsp, nodeCurrentCity, 0)) {
        double cost = node->cost + tsp->roadCosts[nodeCurrentCity][0];

        if (cost < tsp->bestTourCost) {
            res = true;
            if (tsp->solution != NULL)
                tspNodeDestroy(tsp->solution);

            tsp->solution = tspNodeCreate(cost, _calculateLb(tsp, node, 0), node->length + 1, 0);
            tspNodeCopyTour(node, tsp->solution);

            #pragma omp atomic write
            tsp->bestTourCost = cost;
        }
    }
    return res;
}

void visitNeighbours(tsp_t* tsp, tspNode_t* node, size_t nodeCurrentCity) {
    size_t cityNumber;
    tspNode_t* nextNode;
    double lb, cost;
    // #pragma omp for private(cityNumber, lb, cost, nextNode)
    for (cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (_isNeighbour(tsp, nodeCurrentCity, cityNumber) && !_isCityInTour(node, cityNumber)) {
            lb = _calculateLb(tsp, node, cityNumber);

            if (lb > tsp->bestTourCost) continue;

            cost = node->cost + tsp->roadCosts[nodeCurrentCity][cityNumber];
            nextNode = tspNodeCreate(cost, lb, node->length + 1, cityNumber);
            tspNodeCopyTour(node, nextNode);

            #pragma omp critical(queue)
            queuePush(&tsp->queue, nextNode);
            #pragma omp task
            processNode(tsp);
        }
    }
}

void processNode(tsp_t* tsp) {
    // printf("NumThreads %d\n", omp_get_thread_num());
    tspNode_t* node = NULL;

    #pragma omp critical(queue)
    node = queuePop(&tsp->queue);

    if (!verifyNode(tsp, node)) {
        return;
    }

    size_t nodeCurrentCity;
        
    nodeCurrentCity = tspNodeCurrentCity(node);

    if (updateBestTour(tsp, node, nodeCurrentCity));
    else {
        visitNeighbours(tsp, node, nodeCurrentCity);
    }
    tspNodeDestroy(node);
}

void tspSolve(tsp_t* tsp) {

    tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(tsp), 1, 0);
    queuePush(&tsp->queue, startNode);

    #pragma omp parallel
    #pragma omp single    
    #pragma omp task
    {
        processNode(tsp);
    }

}
