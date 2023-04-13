#include "tspSolver.h"
#include "tspApi.h"
#include "tspNode.h"
#include "utils/queue.h"
#include <math.h>
#include <mpi.h>
#include <time.h>

typedef struct {
    const tsp_t* tsp;
    tspApi_t* api;
    tspSolution_t* solution;
    priorityQueue_t* queue;
} tspSolverData_t;

tspSolution_t* tspSolutionCreate(double maxTourCost) {
    tspSolution_t* solution = (tspSolution_t*)malloc(sizeof(tspSolution_t));
    solution->hasSolution = false;
    solution->cost = maxTourCost;
    solution->priority = maxTourCost * MAX_CITIES + MAX_CITIES - 1;
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution); }

static inline bool _isBetterSolution(tspSolution_t* oldSolution, tspSolution_t* newSolution) {
    return newSolution->priority < oldSolution->priority;
}

static void _copySolution(const tsp_t* tsp, tspSolution_t* srcSolution, tspSolution_t* destSolution) {
    destSolution->hasSolution = srcSolution->hasSolution;
    destSolution->cost = srcSolution->cost;
    destSolution->priority = srcSolution->priority;
    for (int i = 0; i < tsp->nCities; i++)
        destSolution->tour[i] = srcSolution->tour[i];
}

static int __tspNodeCmpFun(void* el1, void* el2) {
    tspNode_t* node1 = (tspNode_t*)el1;
    tspNode_t* node2 = (tspNode_t*)el2;
    return (node2->priority < node1->priority ? 1 : 0);
}

static void __tspNodeDestroyFun(void* el) {
    tspNode_t* node = (tspNode_t*)el;
    tspNodeDestroy(node);
}

static inline bool _isCityInTour(const tspNode_t* node, int cityNumber) {
    return node->visited & (0x00000001 << cityNumber);
}

static tspNode_t* _getNextNode(priorityQueue_t* queue, double solutionPriority) {
    tspNode_t* node = queuePop(queue);
    if (node != NULL && node->priority > solutionPriority) {
        tspNodeDestroy(node);
        return NULL;
    }
    return node;
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
    if (priority < solution->priority) {
        tspNodeCopyTour(finalNode, solution->tour);
        solution->hasSolution = true;
        solution->cost = cost;
        solution->priority = cost * MAX_CITIES + solution->tour[tsp->nCities - 1];

        for (int i = 0; i < solverData->api->nProcs; i++) {
            if (i == solverData->api->procId)
                continue;
            MPI_Send(solution, 1, solverData->api->solution_t, i, MPI_TAG_SOLUTION, MPI_COMM_WORLD);
        }
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
            queuePush(solverData->queue, nextNode);
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

void _recvSolution(tspSolverData_t* solverData, MPI_Status* status) {
    tspSolution_t recvSolution;
    MPI_Status statusSolution;
    MPI_Recv(&recvSolution, 1, solverData->api->solution_t, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD,
             &statusSolution);

    if (_isBetterSolution(solverData->solution, &recvSolution))
        _copySolution(solverData->tsp, &recvSolution, solverData->solution);
}
void _recvNode(tspSolverData_t* solverData, MPI_Status* status) {
    tspNode_t* node;
    MPI_Status statusNode;
    node = tspNodeCreate(0, 0, 1, 0);
    MPI_Recv(node, 1, solverData->api->node_t, status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD, &statusNode);
    queuePush(solverData->queue, node);
}

void _singleProcSolve(tspSolverData_t* solverData) {
    tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(solverData->tsp), 1, 0);
    _processNode(solverData, startNode);
    tspNodeDestroy(startNode);

    while (true) {
        tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);
        if (node == NULL)
            break;
        _processNode(solverData, node);
        tspNodeDestroy(node);
    }
}

void _multipleProcSolve(tspSolverData_t* solverData) {
    if (solverData->api->procType == PROCTYPE_MASTER) {
        // Initialization
        int flag;
        int next = 1;
        bool isInit = true;
        bool temp = false;
        bool isTerminated[solverData->api->nProcs];
        memset(isTerminated, false, solverData->api->nProcs * sizeof(bool));

        tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(solverData->tsp), 1, 0);
        _processNode(solverData, startNode);
        tspNodeDestroy(startNode);

        int numCycles = (solverData->api->nProcs + 2 - 1) / 2;
        for (int i = 0; i < numCycles; i++) {
            tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);
            if (node == NULL)
                break;
            _processNode(solverData, node);
            tspNodeDestroy(node);
        }

        for (int i = 0; i < (2 * solverData->api->nProcs); i++) {
            tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);
            if (node == NULL)
                break;
            MPI_Send(node, 1, solverData->api->node_t, next, MPI_TAG_NODE, MPI_COMM_WORLD);
            next = (next + 1) % solverData->api->nProcs;
            if (next == 0)
                next = 1; //(id + 1) % nprocs;
            tspNodeDestroy(node);
        }

        for (int i = 1; i < solverData->api->nProcs; i++)
            MPI_Send(&isInit, 1, MPI_C_BOOL, i, MPI_TAG_INIT, MPI_COMM_WORLD);

        // Works as special Process
        next = 1;

        while (true) {
            flag = false;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                if (status.MPI_TAG == MPI_TAG_SOLUTION)
                    _recvSolution(solverData, &status);
                else if (status.MPI_TAG == MPI_TAG_NODE)
                    _recvNode(solverData, &status);
                else if (status.MPI_TAG == MPI_TAG_ASK_NODE) {
                    MPI_Recv(&temp, 1, MPI_C_BOOL, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, NULL);
                    tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);
                    if (node == NULL) {
                        MPI_Send(&temp, 1, MPI_C_BOOL, status.MPI_SOURCE, MPI_TAG_TODO1, MPI_COMM_WORLD);
                        isTerminated[status.MPI_SOURCE] = true;
                        bool terminated = true;
                        for (int i = 1; i < solverData->api->nProcs; i++)
                            if (!isTerminated[i])
                                terminated = false;
                        if (terminated)
                            break;
                    } else {
                        MPI_Send(node, 1, solverData->api->node_t, status.MPI_SOURCE, MPI_TAG_TODO2, MPI_COMM_WORLD);
                    }
                }
            }
            tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);

            if (node == NULL)
                continue;

            _processNode(solverData, node);
            tspNodeDestroy(node);
        }

    } else {
        // All Other processes

        int flag;
        bool isInit = false; // can only ask nodes after initialization has finished
        int MASTER_SOURCE = 0;
        bool askedMaster = false; // used only for ask once to master for a node
        bool temp = false;

        while (true) {
            flag = false;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                if (status.MPI_TAG == MPI_TAG_SOLUTION)
                    _recvSolution(solverData, &status);
                else if (status.MPI_TAG == MPI_TAG_NODE)
                    _recvNode(solverData, &status);
                else if (status.MPI_TAG == MPI_TAG_TODO2) {
                    _recvNode(solverData, &status);
                    askedMaster = false;
                } else if (status.MPI_TAG == MPI_TAG_TODO1) {
                    MPI_Status tempStatus;
                    MPI_Recv(&temp, 1, MPI_C_BOOL, MASTER_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &tempStatus);
                    break;
                } else if (status.MPI_TAG == MPI_TAG_INIT) {
                    MPI_Status tempStatus;
                    MPI_Recv(&isInit, 1, MPI_C_BOOL, 0, status.MPI_TAG, MPI_COMM_WORLD, &tempStatus);
                }
            }
            tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);

            if (node == NULL) {
                if (!askedMaster && isInit) {
                    // ask master for a new node
                    MPI_Send(&temp, 1, MPI_C_BOOL, MASTER_SOURCE, MPI_TAG_ASK_NODE, MPI_COMM_WORLD);
                    askedMaster = true;
                }
                continue;
            } else {
                _processNode(solverData, node);
                tspNodeDestroy(node);
            }
        }
    }
}

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    tspSolverData_t solverData;
    solverData.tsp = tsp;
    solverData.api = tspApiCreate();
    solverData.solution = tspSolutionCreate(maxTourCost);
    solverData.queue = queueCreate(__tspNodeCmpFun);

    tspApiInit(solverData.api);

    if (solverData.api->nProcs == 1) {
        _singleProcSolve(&solverData);
    } else {
        _multipleProcSolve(&solverData);
    }

    int procId = solverData.api->procId;
    tspApiTerminate(solverData.api);
    tspApiDestroy(solverData.api);
    queueDestroy(solverData.queue, __tspNodeDestroyFun);
    if (procId) {
        tspSolutionDestroy(solverData.solution);
        exit(EXIT_SUCCESS);
    } else {
        return solverData.solution;
    }
}