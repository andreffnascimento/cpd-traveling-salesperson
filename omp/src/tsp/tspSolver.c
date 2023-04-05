#include "tspSolver.h"
#include "tspLoadBalancer.h"
#include "tspNode.h"
#include <math.h>
#include <omp.h>

typedef struct {
    const tsp_t* tsp;
    tspSolution_t* solution;
    tspLoadBalancer_t* loadBalancer;
} tspSolverData_t;

tspSolution_t* tspSolutionCreate(double maxTourCost) {
    tspSolution_t* solution = (tspSolution_t*)malloc(sizeof(tspSolution_t));
    solution->hasSolution = false;
    solution->cost = maxTourCost;
    solution->priority = maxTourCost * MAX_CITIES + MAX_CITIES - 1;
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution); }

static inline bool _isCityInTour(const tspNode_t* node, int cityNumber) {
    return node->visited & (0x00000001 << cityNumber);
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

static void _updateBestTour(tspSolverData_t* tspSolverData, const tspNode_t* finalNode) {
    const tsp_t* tsp = tspSolverData->tsp;
    tspSolution_t* solution = tspSolverData->solution;
    int nodeCurrentCity = tspNodeCurrentCity(finalNode);
    double cost = finalNode->cost + tsp->roadCosts[nodeCurrentCity][0];
    double priority = cost * MAX_CITIES + nodeCurrentCity;
#pragma omp critical(solution)
    if (priority < solution->priority) {
        tspNodeCopyTour(finalNode, solution->tour);
        solution->hasSolution = true;
        solution->cost = cost;
        solution->priority = cost * MAX_CITIES + solution->tour[tsp->nCities - 1];
    }
}

static void _visitNeighbors(tspSolverData_t* tspSolverData, const tspNode_t* parent) {
    const tsp_t* tsp = tspSolverData->tsp;
    int parentCurrentCity = tspNodeCurrentCity(parent);
    for (int cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (tspIsNeighbour(tsp, parentCurrentCity, cityNumber) && !_isCityInTour(parent, cityNumber)) {
            double lb = _calculateLb(tsp, parent, cityNumber);
            if (lb > tspSolverData->solution->cost)
                continue;
            double cost = parent->cost + tsp->roadCosts[parentCurrentCity][cityNumber];
            tspNode_t* nextNode = tspNodeExtend(parent, cost, lb, cityNumber);
            tspLoadBalancerPush(tspSolverData->loadBalancer, nextNode);
        }
    }
}

static void _processNode(tspSolverData_t* tspSolverData, tspNode_t* node) {
    const tsp_t* tsp = tspSolverData->tsp;
    if ((node->length == tsp->nCities) && tspIsNeighbour(tsp, tspNodeCurrentCity(node), 0))
        _updateBestTour(tspSolverData, node);
    else
        _visitNeighbors(tspSolverData, node);
}

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    tspSolverData_t tspSolverData;

#pragma omp parallel num_threads(6)
    {
#pragma omp single
        {
            tspSolverData.tsp = tsp;
            tspSolverData.solution = tspSolutionCreate(maxTourCost);
            tspSolverData.loadBalancer = tspLoadBalancerCreate(omp_get_num_threads());
            tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(tsp), 1, 0);
            _processNode(&tspSolverData, startNode);
            tspNodeDestroy(startNode);
        }

        while (true) {
            tspNode_t* node = tspLoadBalancerPop(tspSolverData.loadBalancer, &tspSolverData.solution->priority);
            if (node == NULL)
                break;
            _processNode(&tspSolverData, node);
            tspNodeDestroy(node);
        }
    }

    tspLoadBalancerDestroy(tspSolverData.loadBalancer);
    return tspSolverData.solution;
}
