#include "tspSolver.h"
#include "tspNode.h"
#include "utils/queue.h"
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define INITIAL_CONTROL_DELAY 1
#define CONTROL_DELAY_MULTIPLIER(DELAY) DELAY * 1.2

typedef struct {
    const tsp_t* tsp;
    tspApi_t* api;
    tspSolution_t* solution;
    priorityQueue_t* queue;

    bool controlBlock;
    double controlTime;
    double controlDelay;
    tspApiControl_t controlMsgIn;
    tspApiControl_t controlMsgOut;
    MPI_Request controlRequest;
} tspSolverData_t;

tspSolution_t* tspSolutionCreate(double maxTourCost) {
    tspSolution_t* solution = (tspSolution_t*)malloc(sizeof(tspSolution_t));
    solution->hasSolution = false;
    solution->cost = maxTourCost;
    solution->priority = maxTourCost * MAX_CITIES + MAX_CITIES - 1;
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution); }

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

static void _processStartNode(tspSolverData_t* solverData) {
    if (solverData->api->procType == PROCTYPE_MASTER) {
        tspNode_t* startNode = tspNodeCreate(0, _calculateInitialLb(solverData->tsp), 1, 0);
        _processNode(solverData, startNode);
        tspNodeDestroy(startNode);
    }
}

static bool _getControlInfo(tspSolverData_t* solverData) {
    tspApiControl_t* controlMsg = &solverData->controlMsgIn;
    MPI_Wait(&solverData->controlRequest, NULL);
    if (!controlMsg->running)
        return false;

    solverData->controlDelay = controlMsg->delay;
    if (controlMsg->solutionPriority < solverData->solution->priority) {
        solverData->solution->hasSolution = controlMsg->hasSolution;
        solverData->solution->cost = controlMsg->solutionCost;
        solverData->solution->priority = controlMsg->solutionPriority;
        for (int i = 0; i < MAX_CITIES; i++)
            solverData->solution->tour[i] = controlMsg->solutionTour[i];
    }

    for (int i = 0; i < controlMsg->nNodes; i++) {
        double cost = controlMsg->costs[i];
        double lb = controlMsg->lbs[i];
        int length = controlMsg->lengths[i];
        int currentCity = controlMsg->tours[i][controlMsg->lengths[i] - 1];
        tspNode_t* node = tspNodeCreate(cost, lb, length, currentCity);
        for (int j = 0; j < controlMsg->lengths[i]; j++)
            node->tour[j] = controlMsg->tours[i][j];
        node->visited = controlMsg->visited[i];
        queuePush(solverData->queue, node);
    }

    return true;
}

static void _sendControlInfo(tspSolverData_t* solverData, bool running) {
    tspApiControl_t* controlMsg = &solverData->controlMsgOut;
    controlMsg->running = running;
    controlMsg->hasSolution = solverData->solution->hasSolution;
    controlMsg->solutionCost = solverData->solution->cost;
    controlMsg->solutionPriority = solverData->solution->priority;
    for (int i = 0; i < MAX_CITIES; i++)
        controlMsg->solutionTour[i] = solverData->solution->tour[i];

    controlMsg->nNodes = 0;
    for (int i = 0; i < MAX_CONTROL_NODES; i++) {
        tspNode_t* node = _getNextNode(solverData->queue, solverData->solution->priority);
        if (node == NULL)
            break;

        controlMsg->nNodes++;
        controlMsg->costs[i] = node->cost;
        controlMsg->lbs[i] = node->lb;
        controlMsg->priorities[i] = node->priority;
        controlMsg->lengths[i] = node->length;
        controlMsg->visited[i] = node->visited;
        tspNodeCopyTour(node, controlMsg->tours[i]);
    }

    MPI_Request gatherRequest;
    tspApiControl_t* gatheredControlMsgs = (tspApiControl_t*)malloc(solverData->api->nProcs * sizeof(tspApiControl_t));
    MPI_Igather(controlMsg, 1, solverData->api->control_t, gatheredControlMsgs, 1, solverData->api->control_t, 0,
                MPI_COMM_WORLD, &gatherRequest);
    MPI_Irecv(&solverData->controlMsgIn, 1, solverData->api->control_t, 0, MPI_TAG_CONTROL, MPI_COMM_WORLD,
              &solverData->controlRequest);
    if (solverData->api->procType != PROCTYPE_MASTER)
        return;

    MPI_Wait(&gatherRequest, NULL);

    int nGatheredNodes = gatheredControlMsgs[0].nNodes;;
    int bestSolutionProc = 0;
    double bestSolutionPriority = gatheredControlMsgs[0].solutionPriority;
    for (int i = 1; i < solverData->api->nProcs; i++) {
        nGatheredNodes += gatheredControlMsgs[i].nNodes;
        if (gatheredControlMsgs[i].solutionPriority < bestSolutionPriority) {
            bestSolutionProc = i;
            bestSolutionPriority = gatheredControlMsgs[i].solutionPriority;
        }
    }

    int* gatheredNodeLocs = NULL;
    if (nGatheredNodes > 0) {
        gatheredNodeLocs = (int*)malloc(nGatheredNodes * 2 * sizeof(int));
        for (int i = 0, j = 0, k = 0; i < solverData->api->nProcs; i++, k = 0) {
            gatheredNodeLocs[j++] = i;
            gatheredNodeLocs[j++] = k++;
        }
    }


    tspApiControl_t controlMsgOut;
    controlMsgOut.running = false;
    controlMsgOut.delay = CONTROL_DELAY_MULTIPLIER(solverData->controlDelay);
    for (int i = 0; i < solverData->api->nProcs; i++)
        controlMsgOut.running = controlMsgOut.running || gatheredControlMsgs[i].running;
    controlMsgOut.hasSolution = gatheredControlMsgs[bestSolutionProc].hasSolution;
    controlMsgOut.solutionCost = gatheredControlMsgs[bestSolutionProc].solutionCost;
    controlMsgOut.solutionPriority = gatheredControlMsgs[bestSolutionProc].solutionPriority;
    for (int i = 0; i < MAX_CITIES; i++)
        controlMsgOut.solutionTour[i] = gatheredControlMsgs[bestSolutionProc].solutionTour[i];

    int nodeId = 0;
    for (int i = 0; i < solverData->api->nProcs; i++) {
        controlMsgOut.nNodes = nGatheredNodes / solverData->api->nProcs;
        for (int j = 0; j < nGatheredNodes / solverData->api->nProcs; j++, nodeId++) {
            int procId = gatheredNodeLocs[nodeId];
            int offset = gatheredNodeLocs[nodeId + 1];
            controlMsgOut.costs[j] = gatheredControlMsgs[procId].costs[offset];
            controlMsgOut.lbs[j] = gatheredControlMsgs[procId].lbs[offset];
            controlMsgOut.priorities[j] = gatheredControlMsgs[procId].priorities[offset];
            controlMsgOut.lengths[j] = gatheredControlMsgs[procId].lengths[offset];
            for (int k = 0; k < gatheredControlMsgs[procId].lengths[offset]; k++)
                controlMsgOut.tours[j][k] = gatheredControlMsgs[procId].tours[offset][k];
            controlMsgOut.visited[j] = gatheredControlMsgs[procId].visited[offset];
        }

        MPI_Request request;
        MPI_Isend(&controlMsgOut, 1, solverData->api->control_t, i, MPI_TAG_CONTROL, MPI_COMM_WORLD, &request);
    }

    // for (; nodeId < nGatheredNodes; nodeId++) {
    //     int procId = gatheredNodeLocs[nodeId];
    //     int offset = gatheredNodeLocs[nodeId + 1];
    //     double cost = gatheredControlMsgs[procId].costs[offset];
    //     double lb = gatheredControlMsgs[procId].lbs[offset];
    //     int length = gatheredControlMsgs[procId].lengths[offset];
    //     int currentCity = gatheredControlMsgs[procId].tours[offset][gatheredControlMsgs[procId].lengths[offset] - 1];
    //     tspNode_t* node = tspNodeCreate(cost, lb, length, currentCity);
    //     for (int j = 0; j < gatheredControlMsgs[procId].lengths[offset]; j++)
    //         node->tour[j] = gatheredControlMsgs[procId].tours[offset][j];
    //     node->visited = gatheredControlMsgs[procId].visited[offset];
    //     queuePush(solverData->queue, node);
    // }
}

tspSolution_t* tspSolve(tspApi_t* api, const tsp_t* tsp, double maxTourCost) {
    tspSolverData_t solverData;
    solverData.tsp = tsp;
    solverData.api = api;
    solverData.solution = tspSolutionCreate(maxTourCost);
    solverData.queue = queueCreate(__tspNodeCmpFun);
    solverData.controlBlock = false;
    solverData.controlTime = -omp_get_wtime();
    solverData.controlDelay = INITIAL_CONTROL_DELAY;
    _processStartNode(&solverData);

    while (true) {
        int hasControlRequestArrived = 0;
        MPI_Test(&solverData.controlRequest, &hasControlRequestArrived, NULL);
        if (hasControlRequestArrived != 0) {
            solverData.controlBlock = false;
            solverData.controlTime = -omp_get_wtime();
            if (!_getControlInfo(&solverData))
                break;
        }

        if (!solverData.controlBlock && solverData.controlTime + omp_get_wtime() > solverData.controlDelay) {
            solverData.controlBlock = true;
            _sendControlInfo(&solverData, true);
        }

        tspNode_t* node = _getNextNode(solverData.queue, solverData.solution->priority);
        if (node == NULL) {
            _sendControlInfo(&solverData, false);
            MPI_Wait(&solverData.controlRequest, NULL);
            continue;
        }

        _processNode(&solverData, node);
        tspNodeDestroy(node);
    }

    queueDestroy(solverData.queue, __tspNodeDestroyFun);
    return solverData.solution;
}
