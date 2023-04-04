#include <math.h>
#include <mpi.h>
#include "tspSolver.h"
#include "container.h"
#include "node.h"
#include "utils/queue.h"
#include "mpiHandler.h"

int id, nprocs;
MPI_Datatype MPI_SOLUTION; 

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

static void _copySolution(tspSolution_t* from, tspSolution_t* to) {
    // Does not perform validation of oldSolution
    to->hasSolution = from->hasSolution;
    to->cost = from->cost;
    strcpy(to->tour, from->tour);
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
        if (id) MPI_Send(solution, 1, MPI_SOLUTION, 0, SOLUTION_TAG, MPI_COMM_WORLD);
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

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    Container_t* container = containerCreate();
    tspSolution_t* solution = tspSolutionCreate(maxTourCost);
    priorityQueue_t queue = queueCreate(__tspNodeCompFun);

    ContainerEntry_t* entry;
    Node_t *node;
    int prev = (id - 1);

    MPI_SOLUTION = mpiSolutionDataType();

    if (nprocs == 1) {
        //processStuff
        fprintf(stderr, "Warning nprocs == 1\n");
    }
    else if (!id) {
        // Initialization
        int next = 1;
        bool terminated[nprocs];
        memset(terminated, false, nprocs*sizeof(bool));

        ContainerEntry_t* startEntry = containerGetEntry(container);
        nodeInit(containerGetNode(startEntry), 0, _calculateInitialLb(tsp), 1, 0);
        _processNode(tsp, container, &queue, solution, startEntry);

        while(true) {
            //Distribute nodes across diff processes
            entry = _getNextNode(&queue, solution->cost);
            if (!entry) break;
            node = containerGetNode(entry);
            sendNode(node, next);
            next = (id + 1) % nprocs;
            if ((next + 1) == 0) next = (id + 1) % nprocs;
            containerRemoveEntry(container, entry);
        }


        while (true) {
            //Message Exchanger
            int flag = false;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                if (status.MPI_TAG == SOLUTION_TAG) {
                    tspSolution_t newSolution;
                    MPI_Status newStatus;
                    MPI_Request request;
                    MPI_Recv(&newSolution, 1, MPI_SOLUTION, status.MPI_SOURCE, SOLUTION_TAG, MPI_COMM_WORLD, &newStatus);
                    if (_isBetterSolution(solution, &newSolution)) {
                        _copySolution(&newSolution, solution);
                        MPI_Ibcast(solution, 1, MPI_SOLUTION, 0, MPI_COMM_WORLD, &request);
                        //broadcast
                    }
                }
                else if (status.MPI_TAG == TERMINATED_TAG) {
                    terminated[status.MPI_SOURCE - 1] = true;
                    bool finished = true;
                    for (int i = 1; i < nprocs; i++) {
                        if (!terminated[i]) {
                            finished = false;
                            break;
                        }
                    }
                    if (finished) break;
                }
            }
        }
    }
    else {
        // All Others
        entry = containerGetEntry(container);
        node = containerGetNode(entry);
        recvNode(node, prev); //put the node automatically in the container
        queuePush(&queue, entry);

        tspSolution_t recvSolution;
        while (true) {
            recvSolution.hasSolution = false;
            recvAsyncSolution(&recvSolution, 0);
            if (recvSolution.hasSolution && _isBetterSolution(solution, &recvSolution)) _copySolution(&recvSolution, solution);

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
