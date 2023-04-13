#ifndef __TSP_TSP_API_H__
#define __TSP_TSP_API_H__

#include "include.h"
#include <mpi.h>

typedef enum {
    PROCTYPE_MASTER,
    PROCTYPE_TASK,
} tspApiProcType_t;

typedef struct {
    int nProcs;
    int procId;
    tspApiProcType_t procType;
    MPI_Datatype solution_t;
    MPI_Datatype node_t;
} tspApi_t;

tspApi_t* tspApiCreate();
void tspApiDestroy(tspApi_t* api);

void tspApiInit(tspApi_t* api);
void tspApiTerminate(tspApi_t* api);

#define MPI_TAG_NODE 100
#define MPI_TAG_SOLUTION 101
#define MPI_TAG_PSTATUS 102
#define MPI_TAG_TERMINATED 103
#define MPI_TAG_INIT 104
#define MPI_TAG_ASK_NODE 105
#define MPI_TAG_TODO1 106
#define MPI_TAG_TODO2 107

MPI_Datatype tspApiSolutionDatatype();
MPI_Datatype tspApiNodeDatatype();

#endif //__TSP_TSP_API_H__