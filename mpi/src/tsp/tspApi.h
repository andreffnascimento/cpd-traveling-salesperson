#ifndef __TSP_TSP_API_H__
#define __TSP_TSP_API_H__

#include "include.h"
#include "tsp.h"
#include <mpi.h>

#define MAX_CONTROL_NODES 2
#define MPI_TAG_CONTROL 100

typedef enum {
    PROCTYPE_MASTER,
    PROCTYPE_TASK,
} tspApiProcType_t;

typedef struct {
    int nProcs;
    int procId;
    tspApiProcType_t procType;
    MPI_Datatype problem_t;
    MPI_Datatype control_t;
} tspApi_t;

typedef struct {
    bool running;
    double delay;

    bool hasSolution;
    double solutionCost;
    double solutionPriority;
    char solutionTour[MAX_CITIES];

    int nNodes;
    double costs[MAX_CONTROL_NODES];
    double lbs[MAX_CONTROL_NODES];
    double priorities[MAX_CONTROL_NODES];
    int lengths[MAX_CONTROL_NODES];
    char tours[MAX_CONTROL_NODES][MAX_CITIES];
    unsigned long long visited[MAX_CONTROL_NODES];
} tspApiControl_t;

tspApi_t* tspApiInit();
void tspApiTerminate(tspApi_t* api);

#endif //__TSP_TSP_API_H__