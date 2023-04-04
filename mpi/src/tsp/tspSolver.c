#include <math.h>
#include <mpi.h>
#include "tspSolver.h"
#include "container.h"
#include "node.h"
#include "utils/queue.h"
#include "mpiHandler.h"

tspSolution_t* tspSolutionCreate(double maxTourCost) {
    tspSolution_t* solution = (tspSolution_t*)malloc(sizeof(tspSolution_t));
    solution->hasSolution = false;
    solution->cost = maxTourCost;
    return solution;
}

void tspSolutionDestroy(tspSolution_t* solution) { free(solution); }

static int __tspNodeCompFun(void* el1, void* el2) {
    Node_t* tspNode1 = containerGetNode((ContainerEntry_t*)el1);
    Node_t* tspNode2 = containerGetNode((ContainerEntry_t*)el2);
    return (tspNode2->priority < tspNode1->priority ? 1 : 0);
}

static inline bool _isCityInTour(const Node_t* node, int cityNumber) {
    return node->visited & (0x00000001 << cityNumber);
}

static ContainerEntry_t* _getNextNode(priorityQueue_t* queue, double maxTourCost) {
    ContainerEntry_t* entry = queuePop(queue);
    const Node_t* node = containerGetNode(entry);
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
    //BUG: How to include the indexes here?
}

static void _copySolution(tspSolution_t* oldSolution, tspSolution_t* newSolution) {
    // Does not perform validation of oldSolution
    newSolution->hasSolution = oldSolution->hasSolution;
    newSolution->cost = oldSolution->cost;
    strcpy(newSolution->tour, oldSolution->tour);
}

static void _updateBestTour(const tsp_t* tsp, tspSolution_t* solution, const Node_t* finalNode) {
    int currentCity = nodeCurrentCity(finalNode);
    double cost = finalNode->cost + tsp->roadCosts[currentCity][0];
    bool isNewSolution = !solution->hasSolution || cost < solution->cost ||
                         (cost == solution->cost && currentCity < solution->tour[tsp->nCities - 1]);
    if (isNewSolution) {
        DEBUG(nodePrint(finalNode));
        solution->hasSolution = true;
        solution->cost = cost;
        nodeCopyTour(finalNode, solution->tour);
        sendAsyncSolution(solution, 0);
    }
}

static void _visitNeighbors(const tsp_t* tsp, Container_t* container, priorityQueue_t* queue,
                            tspSolution_t* solution, const Node_t* parent) {
    int parentCurrentCity = nodeCurrentCity(parent);
    for (int cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {
        if (tspIsNeighbour(tsp, parentCurrentCity, cityNumber) && !_isCityInTour(parent, cityNumber)) {
            double lb = _calculateLb(tsp, parent, cityNumber);
            if (lb > solution->cost)
                continue;
            double cost = parent->cost + tsp->roadCosts[parentCurrentCity][cityNumber];
            ContainerEntry_t* nextEntry = containerGetEntry(container);
            nodeExtInit(containerGetNode(nextEntry), parent, cost, lb, cityNumber);
            queuePush(queue, nextEntry);
        }
    }
}

static void _processNode(const tsp_t* tsp, Container_t* container, priorityQueue_t* queue, tspSolution_t* solution,
                         ContainerEntry_t* entry) {
    Node_t* node = containerGetNode(entry);
    DEBUG(nodePrint(node));
    if ((node->length == tsp->nCities) && tspIsNeighbour(tsp, nodeCurrentCity(node), 0))
        _updateBestTour(tsp, solution, node);
    else
        _visitNeighbors(tsp, container, queue, solution, node);
    containerRemoveEntry(container, entry);
}

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost) {
    int id, nprocs;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    Container_t* container = containerCreate();
    tspSolution_t* solution = tspSolutionCreate(maxTourCost);
    priorityQueue_t queue = queueCreate(__tspNodeCompFun);

    ContainerEntry_t* entry;
    Node_t *node;
    int next = (id + 1) % nprocs;
    int prev = (id - 1);

    MPI_Datatype MPI_SOLUTION = mpiSolutionDataType();

    if (!id) {
        prev = nprocs - 1;
        ContainerEntry_t* startEntry = containerGetEntry(container);
        nodeInit(containerGetNode(startEntry), 0, _calculateInitialLb(tsp), 1, 0);
        _processNode(tsp, container, &queue, solution, startEntry);

        while(true) {
            //Distribute nodes across diff processes
            entry = _getNextNode(&queue, solution->cost);
            node = containerGetNode(entry);
            sendAsyncNode(node, next);
            next = (next + 1) % nprocs;
            containerRemoveEntry(container, entry);
        }

    }
    entry = containerGetEntry(container);
    node = containerGetNode(entry);
    recvNode(node, prev); //put the node automatically in the container
    queuePush(&queue, entry);

    if (!id) {
        bool updatedSolution;
        tspSolution_t newSolution;
        bool terminated[nprocs];
        memset(terminated, false, nprocs*sizeof(bool));

        while (true) {
            updatedSolution = false;
            
            // Fetch Processes' solutions
            for (int i = 1; i < nprocs; i++) {
                if (hasMessageToReceive(i, SOLUTION_TAG)) {
                    //TODO: MPI_COUNT 
                    recvAsyncSolution(&newSolution, i);
                    if (_isBetterSolution(solution, &newSolution)) {
                        _copySolution(solution, &newSolution);
                        updatedSolution = true;
                    }
                }
            }

            //Broadcast the best solution
            if (updatedSolution) {
                MPI_Request request;
                MPI_Ibcast(solution, 1, MPI_SOLUTION, 0, MPI_COMM_WORLD, &request);
            }

            // Fetch For Terminated Process
            
        }
    }
    else {
        while (true) {
            entry = _getNextNode(&queue, solution->cost);
            node = containerGetNode(entry);
            
            if (entry == NULL)
                break;

            _processNode(tsp, container, &queue, solution, entry);
        }
    }



    queueDelete(&queue, NULL);
    containerDestroy(container);
    if (id) tspSolutionDestroy(solution);

    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();
    return solution;
}
