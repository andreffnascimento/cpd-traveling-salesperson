#include "tspSolver.h"

#include <math.h>
#include <mpi.h>

#include "mpiHandler.h"
#include "node.h"
#include "utils/queue.h"

typedef struct {
    const tsp_t* tsp;
    tspSolution_t* solution;
    priorityQueue_t* queue;
} tspSolverData_t;

int id, nprocs;
MPI_Datatype MPI_SOLUTION;
MPI_Datatype MPI_NODE;


tspSolution_t* tspSolutionCreate(double maxTourCost) {
    tspSolution_t* solution = (tspSolution_t*)malloc(sizeof(tspSolution_t));
    solution->hasSolution = false;
    solution->cost = maxTourCost;
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution); }

static int __tspNodeCmpFun(void* el1, void* el2) {
    Node_t* tspNode1 = (Node_t*)el1;
    Node_t* tspNode2 = (Node_t*)el2;
    return (tspNode2->priority < tspNode1->priority ? 1 : 0);
}

static void __tspNodeDestroyFun(void* el) {
    Node_t* node = (Node_t*)el;
    nodeDestroy(node);
}

static inline bool _isCityInTour(const Node_t* node, int cityNumber) {
    return node->visited & (0x00000001 << cityNumber);
}

static Node_t* _getNextNode(tspSolverData_t* tspSolverData) {
    Node_t* node = queuePop(tspSolverData->queue);
    if (node != NULL && node->lb >= tspSolverData->solution->cost) {
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

static double _calculateLb(const tsp_t* tsp, const Node_t* node, int nextCity) {
    int currentCity = nodeCurrentCity(node);
    double min1From = tspMinCost(tsp, currentCity, TSP_MIN_COSTS_1);
    double min2From = tspMinCost(tsp, currentCity, TSP_MIN_COSTS_2);
    double min1To = tspMinCost(tsp, nextCity, TSP_MIN_COSTS_1);
    double min2To = tspMinCost(tsp, nextCity, TSP_MIN_COSTS_2);
    double costFromTo = tsp->roadCosts[currentCity][nextCity];
    double costFrom = (costFromTo >= min2From) ? min2From : min1From;
    double costTo = (costFromTo >= min2To) ? min2To : min1To;
    return node->lb + costFromTo - (costFrom + costTo) / 2;
}

static bool _isBetterSolution(tspSolution_t* oldSolution, tspSolution_t* newSolution) {
    return !oldSolution->hasSolution || newSolution->cost < oldSolution->cost;
    // BUG: How to include the indexes here?
}

static void _copySolution(const tsp_t* tsp, tspSolution_t* from, tspSolution_t* to) {
    // Does not perform validation of oldSolution
    to->hasSolution = from->hasSolution;
    to->cost = from->cost;

    for (int i = 0; i < tsp->nCities; i++)
        to->tour[i] = from->tour[i];
}

static void _updateBestTour(tspSolverData_t* tspSolverData, const Node_t* finalNode) {
    const tsp_t* tsp = tspSolverData->tsp;
    tspSolution_t* solution = tspSolverData->solution;

    int currentCity = nodeCurrentCity(finalNode);
    double cost = finalNode->cost + tsp->roadCosts[currentCity][0];
    bool isNewSolution = !solution->hasSolution || cost < solution->cost ||
                         (cost == solution->cost && currentCity < solution->tour[tsp->nCities - 1]);

    if (isNewSolution) {
        solution->hasSolution = true;
        solution->cost = cost;
        nodeCopyTour(finalNode, solution->tour);
        if (id) {
            for (int i = 0; i < nprocs; i++) {
                if (i == id) continue;
                MPI_Send(solution, 1, MPI_SOLUTION, i, SOLUTION_TAG, MPI_COMM_WORLD);
            }
        }
    }
}

static void _visitNeighbors(tspSolverData_t* tspSolverData, const Node_t* parent) {
    const tsp_t* tsp = tspSolverData->tsp;
    int parentCurrentCity = nodeCurrentCity(parent);
    for (int cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (tspIsNeighbour(tsp, parentCurrentCity, cityNumber) && !_isCityInTour(parent, cityNumber)) {
            double lb = _calculateLb(tsp, parent, cityNumber);
            if (lb > tspSolverData->solution->cost)
                continue;
            double cost = parent->cost + tsp->roadCosts[parentCurrentCity][cityNumber];
            Node_t* nextNode = nodeExtend(parent, cost, lb, cityNumber);
            queuePush(tspSolverData->queue, nextNode);
        }
    }
}

static void _processNode(tspSolverData_t* tspSolverData, Node_t* node) {
    const tsp_t* tsp = tspSolverData->tsp;
    if ((node->length == tsp->nCities) && tspIsNeighbour(tsp, nodeCurrentCity(node), 0))
        _updateBestTour(tspSolverData, node);
    else
        _visitNeighbors(tspSolverData, node);
}

void _recvSolution(tspSolverData_t* tspSolverData, int source) {
    tspSolution_t recvSolution;
    MPI_Status statusSolution;
    MPI_Recv(&recvSolution, 1, MPI_SOLUTION, source, SOLUTION_TAG, MPI_COMM_WORLD, &statusSolution);

    if (_isBetterSolution(tspSolverData->solution, &recvSolution)) _copySolution(tspSolverData->tsp, &recvSolution, tspSolverData->solution);
}
void _recvNode(tspSolverData_t* tspSolverData, int source) {
    Node_t* node;
    MPI_Status statusNode;
    node = nodeCreate(0, 0, 1, 0);
    MPI_Recv(node, 1, MPI_NODE, source, NODE_TAG, MPI_COMM_WORLD, &statusNode);
    queuePush(tspSolverData->queue, node);
}

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    MPI_SOLUTION = mpiSolutionDataType();
    MPI_NODE = mpiNodeDataType();

    tspSolverData_t tspSolverData;
    tspSolverData.tsp = tsp;
    tspSolverData.solution = tspSolutionCreate(maxTourCost);
    tspSolverData.queue = queueCreate(__tspNodeCmpFun);
    Node_t* node;

    if (nprocs == 1) {
        Node_t* startNode = nodeCreate(0, _calculateInitialLb(tsp), 1, 0);
        _processNode(&tspSolverData, startNode);
        nodeDestroy(startNode);

        while (true) {
            node = _getNextNode(&tspSolverData);
            if (node == NULL)
                break;
            _processNode(&tspSolverData, node);
            nodeDestroy(node);
        }

        queueDestroy(tspSolverData.queue, __tspNodeDestroyFun);
        MPI_Finalize();
        return tspSolverData.solution;

    } else if (!id) {
        // Initialization
        int next = 1;
        int prev = nprocs - 1;
        bool isProcessing[nprocs];
        bool isTerminated[nprocs];
        memset(isProcessing, true, nprocs * sizeof(bool));
        bool isInit = true;

        Node_t* startNode = nodeCreate(0, _calculateInitialLb(tsp), 1, 0);
        _processNode(&tspSolverData, startNode);
        nodeDestroy(startNode);

        while (true) {
            // Distribute nodes across diff processes
            node = _getNextNode(&tspSolverData);
            if (node == NULL) break;

            MPI_Send(node, 1, MPI_NODE, next, NODE_TAG, MPI_COMM_WORLD);

            next = (next + 1) % nprocs;
            if (next == 0) next = 1;  //(id + 1) % nprocs;

            nodeDestroy(node);
        }

        for (int i = 1; i < nprocs; i++)
            MPI_Send(&isInit, 1, MPI_C_BOOL, i, INIT_TAG, MPI_COMM_WORLD);

        //Works as special Process
        int flag;
        next = 1;

        while (true) {
            flag = false;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                if (status.MPI_TAG == SOLUTION_TAG) _recvSolution(&tspSolverData, status.MPI_SOURCE);
                else if (status.MPI_TAG == NODE_TAG) _recvNode(&tspSolverData, status.MPI_SOURCE);
                else if (status.MPI_TAG == PROCESSING_STATUS_TAG) {
                    bool isProcProcessing;
                    MPI_Status processingStatus;
                    MPI_Recv(&isProcProcessing, 1, MPI_C_BOOL, status.MPI_SOURCE, PROCESSING_STATUS_TAG, MPI_COMM_WORLD, &processingStatus);
                    isProcessing[status.MPI_SOURCE] = isProcProcessing;

                    if (!isProcProcessing) {
                        bool finished = true;
                        for (int i = 1; i < nprocs; i++) {
                            if (isProcessing[i]) {
                                finished = false;
                                break;
                            }
                        }
                        if (finished) {
                            for (int i = 1; i < nprocs; i++)
                                MPI_Send(&finished, 1, MPI_C_BOOL, i, TERMINATED_TAG, MPI_COMM_WORLD);
                        } 
                    }
                }
                else if (status.MPI_TAG == TERMINATED_TAG) {
                    bool _isTerminated, finished = true;
                    MPI_Recv(&_isTerminated, 1, MPI_C_BOOL, status.MPI_SOURCE, TERMINATED_TAG, MPI_COMM_WORLD, NULL);
                    isTerminated[status.MPI_SOURCE] = _isTerminated;
                    finished = true;
                    for (int i = 1; i < nprocs; i++)
                        if (!isTerminated[i]) finished = false;
    
                    if (finished) break;

                }
            }
            node = _getNextNode(&tspSolverData);

            if (!node) continue;

            _processNode(&tspSolverData, node);
        }

        
    } else {
        // All Other processes
        //  Zero needs to have the latest solution

        int flag;
        bool isInit = false;
        bool isProcessing = true;
        bool isTerminating = false;
        int next = id + 1;
        int prev = id - 1;
        int sender = 0;
        if (id == (nprocs - 1)) next = 0;


        while (true) {
            flag = false;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                if (status.MPI_TAG == SOLUTION_TAG) {
                    _recvSolution(&tspSolverData, status.MPI_SOURCE);
                } 
                else if (status.MPI_TAG == NODE_TAG) {
                    if (!isProcessing && isInit) {
                        isProcessing = true;
                        MPI_Send(&isProcessing, 1, MPI_C_BOOL, 0, PROCESSING_STATUS_TAG, MPI_COMM_WORLD);
                    }
                    isProcessing = true;
                    _recvNode(&tspSolverData, status.MPI_SOURCE);
                } 
                else if (status.MPI_TAG == TERMINATED_TAG) {
                    bool temp;
                    MPI_Recv(&temp, 1, MPI_C_BOOL, 0, TERMINATED_TAG, MPI_COMM_WORLD, NULL);

                    isTerminating = true;
                    if (isProcessing) printf("[Process %d] termination started while processing\n", id);
                    continue;
                }
                else if (status.MPI_TAG == INIT_TAG) {
                    MPI_Status statusInit;
                    MPI_Recv(&isInit, 1, MPI_C_BOOL, 0, INIT_TAG, MPI_COMM_WORLD, &statusInit);
                }
            }
            node = _getNextNode(&tspSolverData);

            if (node == NULL) {
                if (isTerminating) {
                    MPI_Send(&isTerminating, 1, MPI_C_BOOL, 0, TERMINATED_TAG, MPI_COMM_WORLD);
                    break;
                }
                if (isProcessing && isInit) {
                    isProcessing = false;
                    MPI_Send(&isProcessing, 1, MPI_C_BOOL, 0, PROCESSING_STATUS_TAG, MPI_COMM_WORLD);
                }
                continue;
            }
            else {
                if (!isProcessing && isInit) {
                    isProcessing = true;
                    MPI_Send(&isProcessing, 1, MPI_C_BOOL, 0, PROCESSING_STATUS_TAG, MPI_COMM_WORLD);
                }
                _processNode(&tspSolverData, node);
                node = _getNextNode(&tspSolverData);

            }
        }
    }

    // queueDestroy(tspSolverData.queue, __tspNodeDestroyFun);
    // if (id) tspSolutionDestroy(tspSolverData.solution);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    if (id) exit(0);
    return tspSolverData.solution;
}