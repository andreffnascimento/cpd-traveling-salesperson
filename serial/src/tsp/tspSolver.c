#include "tspSolver.h"
#include "tspContainer.h"
#include "tspNode.h"
#include "utils/queue.h"
#include <math.h>

tspSolution_t* tspSolutionCreate(double maxTourCost) {
    tspSolution_t* solution = (tspSolution_t*)malloc(sizeof(tspSolution_t));
    solution->hasSolution = false;
    solution->cost = maxTourCost;
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution); }

static int __tspNodeCompFun(void* el1, void* el2) {
    tspNode_t* tspNode1 = tspContainerEntryVal((tspContainerEntry_t*)el1);
    tspNode_t* tspNode2 = tspContainerEntryVal((tspContainerEntry_t*)el2);
    return (tspNode2->priority < tspNode1->priority ? 1 : 0);
}

// static void __queueDeleteFun(void* el) {
//     tspNode_t* tspNode = (tspNode_t*)el;
//     free(tspNode);
// }

static inline bool _isCityInTour(const tspNode_t* node, int cityNumber) {
    return node->visited & (0x00000001 << cityNumber);
}

static tspContainerEntry_t* _getNextNode(priorityQueue_t* queue, double maxTourCost) {
    tspContainerEntry_t* entry = queuePop(queue);
    const tspNode_t* node = tspContainerEntryVal(entry);
    if (node != NULL && node->lb >= maxTourCost) {
        return NULL;
    }
    return entry;
}

static double _calculateInitialLb(const tsp_t* tsp) {
    double sum = 0.0;
    for (int i = 0; i < tsp->nCities; i++)
        sum += tspMinCost(tsp, i, TSP_MIN_COSTS_1) + tspMinCost(tsp, i, TSP_MIN_COSTS_2);
    return sum / 2;
}

static double _calculateLb(const tsp_t* tsp, const tspNode_t* node, int nextCity) {
    int nodeCurrentCity = tspNodeCurrentCity(node);
    double min1From = tspMinCost(tsp, nodeCurrentCity, TSP_MIN_COSTS_1);
    double min2From = tspMinCost(tsp, nodeCurrentCity, TSP_MIN_COSTS_2);
    double min1To = tspMinCost(tsp, nextCity, TSP_MIN_COSTS_1);
    double min2To = tspMinCost(tsp, nextCity, TSP_MIN_COSTS_2);
    double costFromTo = tsp->roadCosts[nodeCurrentCity][nextCity];
    double costFrom = (costFromTo >= min2From) ? min2From : min1From;
    double costTo = (costFromTo >= min2To) ? min2To : min1To;
    return node->lb + costFromTo - (costFrom + costTo) / 2;
}

static void _updateBestTour(const tsp_t* tsp, tspSolution_t* solution, const tspNode_t* finalNode) {
    int nodeCurrentCity = tspNodeCurrentCity(finalNode);
    double cost = finalNode->cost + tsp->roadCosts[nodeCurrentCity][0];
    bool isNewSolution = !solution->hasSolution || cost < solution->cost ||
                         (cost == solution->cost && nodeCurrentCity < solution->tour[tsp->nCities - 1]);
    if (isNewSolution) {
        DEBUG(tspNodePrint(finalNode));
        solution->hasSolution = true;
        solution->cost = cost;
        tspNodeCopyTour(finalNode, solution->tour);
    }
}

static void _visitNeighbors(const tsp_t* tsp, tspContainer_t* container, priorityQueue_t* queue,
                            tspSolution_t* solution, const tspNode_t* parent) {
    int parentCurrentCity = tspNodeCurrentCity(parent);
    for (int cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (tspIsNeighbour(tsp, parentCurrentCity, cityNumber) && !_isCityInTour(parent, cityNumber)) {
            double lb = _calculateLb(tsp, parent, cityNumber);
            if (lb > solution->cost)
                continue;
            double cost = parent->cost + tsp->roadCosts[parentCurrentCity][cityNumber];
            tspContainerEntry_t* nextEntry = tspContainerGetEntry(container);
            tspNodeExtInit(tspContainerEntryVal(nextEntry), parent, cost, lb, cityNumber);
            queuePush(queue, nextEntry);
        }
    }
}

static void _processNode(const tsp_t* tsp, tspContainer_t* container, priorityQueue_t* queue, tspSolution_t* solution,
                         tspContainerEntry_t* entry) {
    tspNode_t* node = tspContainerEntryVal(entry);
    DEBUG(tspNodePrint(node));
    if ((node->length == tsp->nCities) && tspIsNeighbour(tsp, tspNodeCurrentCity(node), 0))
        _updateBestTour(tsp, solution, node);
    else
        _visitNeighbors(tsp, container, queue, solution, node);
    tspContainerRemoveEntry(container, entry);
}

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    tspContainer_t* container = tspContainerCreate();
    tspSolution_t* solution = tspSolutionCreate(maxTourCost);
    priorityQueue_t queue = queueCreate(__tspNodeCompFun);

    tspContainerEntry_t* startEntry = tspContainerGetEntry(container);
    tspNodeInit(tspContainerEntryVal(startEntry), 0, _calculateInitialLb(tsp), 1, 0);
    _processNode(tsp, container, &queue, solution, startEntry);

    while (true) {
        tspContainerEntry_t* entry = _getNextNode(&queue, solution->cost);
        if (entry == NULL)
            break;
        _processNode(tsp, container, &queue, solution, entry);
    }

    queueDelete(&queue, NULL);
    tspContainerDestroy(container);
    return solution;
}
