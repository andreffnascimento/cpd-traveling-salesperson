#include "tspSolver.h"
#include "utils/queue.h"
#include <math.h>

tspSolution_t tspSolutionCreate(size_t nCities, double maxTourCost) {
    tspSolution_t solution;
    solution.hasSolution = false;
    solution.cost = maxTourCost;
    solution.tour = (size_t*)malloc(nCities * sizeof(size_t));
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution->tour); }

static int __tspNodeCompFun(void* el1, void* el2) {
    tspNode_t* tspNode1 = (tspNode_t*)el1;
    tspNode_t* tspNode2 = (tspNode_t*)el2;
    return (tspNode2->lb < tspNode1->lb ||
            ((tspNode2->lb == tspNode1->lb && tspNodeCurrentCity(tspNode2) < tspNodeCurrentCity(tspNode1))))
               ? 1
               : 0;
}

static void __queueDeleteFun(void* el) {
    tspNode_t* tspNode = (tspNode_t*)el;
    free(tspNode);
}

static inline bool _isNeighbour(const tsp_t* tsp, size_t cityA, size_t cityB) {
    return tsp->roadCosts[cityA][cityB] != NONEXISTENT_ROAD_VALUE;
}

static inline bool _isCityInTour(const tspNode_t* node, size_t cityNumber) {
    return node->visited & (0x00000001 << cityNumber);
}

static tspNode_t* _getNextNode(priorityQueue_t* queue, double maxTourCost) {
    tspNode_t* node = queuePop(queue);
    if (node != NULL && node->lb >= maxTourCost) {
        tspNodeDestroy(node);
        return NULL;
    }
    return node;
}

static double _calculateInitialLb(const tsp_t* tsp) {
    double sum = 0.0;
    for (size_t i = 0; i < tsp->nCities; i++) {
        double min1 = INFINITY, min2 = INFINITY;
        for (size_t j = 0; j < tsp->nCities; j++) {
            double cost = tsp->roadCosts[i][j];
            if (_isNeighbour(tsp, i, j)) {
                if (cost < min1) {
                    min2 = min1;
                    min1 = cost;
                } else if (cost < min2)
                    min2 = cost;
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

static void _updateBestTour(const tsp_t* tsp, tspSolution_t* solution, const tspNode_t* finalNode) {
    size_t nodeCurrentCity = tspNodeCurrentCity(finalNode);
    double cost = finalNode->cost + tsp->roadCosts[nodeCurrentCity][0];
    bool isNewSolution = cost < solution->cost || !solution->hasSolution ||
                         (cost == solution->cost && nodeCurrentCity < solution->tour[tsp->nCities - 1]);
    if (isNewSolution) {
        DEBUG(tspNodePrint(finalNode));
        solution->hasSolution = true;
        solution->cost = cost;
        tspNodeCopyTour(finalNode, solution->tour);
    }
}

static void _processNode(const tsp_t* tsp, priorityQueue_t* queue, tspSolution_t* solution, tspNode_t* node);

static void _visitNeighbors(const tsp_t* tsp, priorityQueue_t* queue, tspSolution_t* solution,
                            const tspNode_t* parent) {
    size_t parentCurrentCity = tspNodeCurrentCity(parent);
    tspNode_t* dfsNode = NULL;
    for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (_isNeighbour(tsp, parentCurrentCity, cityNumber) && !_isCityInTour(parent, cityNumber)) {
            double lb = _calculateLb(tsp, parent, cityNumber);
            if (lb > solution->cost)
                continue;
            double cost = parent->cost + tsp->roadCosts[parentCurrentCity][cityNumber];
            tspNode_t* nextNode = tspNodeExtend(parent, cost, lb, cityNumber);
            if (!solution->hasSolution && dfsNode == NULL)
                dfsNode = nextNode;
            else
                queuePush(queue, nextNode);
        }
    }

    if (dfsNode != NULL)
        _processNode(tsp, queue, solution, dfsNode);
}

static void _processNode(const tsp_t* tsp, priorityQueue_t* queue, tspSolution_t* solution, tspNode_t* node) {
    DEBUG(tspNodePrint(node));
    if ((node->length == tsp->nCities) && _isNeighbour(tsp, tspNodeCurrentCity(node), 0))
        _updateBestTour(tsp, solution, node);
    else
        _visitNeighbors(tsp, queue, solution, node);
    tspNodeDestroy(node);
}

tspSolution_t tspSolve(const tsp_t* tsp, double maxTourCost) {
    tspSolution_t solution = tspSolutionCreate(tsp->nCities, maxTourCost);
    priorityQueue_t queue = queueCreate(__tspNodeCompFun);
    tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(tsp), 1, 0);
    queuePush(&queue, startNode);

    while (true) {
        tspNode_t* node = _getNextNode(&queue, solution.cost);
        if (node == NULL)
            break;
        DEBUG(tspNodePrint(node));
        _processNode(tsp, &queue, &solution, node);
    }

    queueDelete(&queue, __queueDeleteFun);
    return solution;
}
