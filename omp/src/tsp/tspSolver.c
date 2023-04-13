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
    int currentCity = tspNodeCurrentCity(node);
    double min1From = tspMinCost(tsp, currentCity, TSP_MIN_COSTS_1);
    double min2From = tspMinCost(tsp, currentCity, TSP_MIN_COSTS_2);
    double min1To = tspMinCost(tsp, nextCity, TSP_MIN_COSTS_1);
    double min2To = tspMinCost(tsp, nextCity, TSP_MIN_COSTS_2);
    double costFromTo = tsp->roadCosts[currentCity][nextCity];
    double costFrom = (costFromTo >= min2From) ? min2From : min1From;
    double costTo = (costFromTo >= min2To) ? min2To : min1To;
    return node->lb + costFromTo - (costFrom + costTo) / 2;
}

static void _updateBestTour(tspSolverData_t* solverData, const tspNode_t* finalNode) {
    const tsp_t* tsp = solverData->tsp;
    tspSolution_t* solution = solverData->solution;
    int currentCity = tspNodeCurrentCity(finalNode);
    double cost = finalNode->cost + tsp->roadCosts[currentCity][0];
    double priority = cost * MAX_CITIES + currentCity;
#pragma omp critical(solution)
    if (priority < solution->priority) {
        tspNodeCopyTour(finalNode, solution->tour);
        solution->hasSolution = true;
        solution->cost = cost;
        solution->priority = cost * MAX_CITIES + solution->tour[tsp->nCities - 1];
    }
}

static void _visitNeighbors(tspSolverData_t* solverData, const tspNode_t* parent) {
    const tsp_t* tsp = solverData->tsp;
    int parentCurrentCity = tspNodeCurrentCity(parent);
    for (int cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (tspIsNeighbour(tsp, parentCurrentCity, cityNumber) && !_isCityInTour(parent, cityNumber)) {
            double lb = _calculateLb(tsp, parent, cityNumber);
            if (lb > solverData->solution->cost)
                continue;
            double cost = parent->cost + tsp->roadCosts[parentCurrentCity][cityNumber];
            tspNode_t* nextNode = tspNodeCreateExt(parent, cost, lb, cityNumber);
            tspLoadBalancerPush(solverData->loadBalancer, nextNode);
        }
    }
}

static void _processNode(tspSolverData_t* solverData, tspNode_t* node) {
    const tsp_t* tsp = solverData->tsp;
    if ((node->length == tsp->nCities) && tspIsNeighbour(tsp, tspNodeCurrentCity(node), 0))
        _updateBestTour(solverData, node);
    else
        _visitNeighbors(solverData, node);
}

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    tspSolverData_t solverData;

#pragma omp parallel num_threads(6)
    {
#pragma omp single
        {
            solverData.tsp = tsp;
            solverData.solution = tspSolutionCreate(maxTourCost);
            solverData.loadBalancer = tspLoadBalancerCreate(omp_get_num_threads());
            tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(tsp), 1, 0);
            _processNode(&solverData, startNode);
            tspNodeDestroy(startNode);
        }

        while (true) {
            tspNode_t* node = tspLoadBalancerPop(solverData.loadBalancer, &solverData.solution->priority);
            if (node == NULL)
                break;
            _processNode(&solverData, node);
            tspNodeDestroy(node);
        }
    }

    tspLoadBalancerDestroy(solverData.loadBalancer);
    return solverData.solution;
}
