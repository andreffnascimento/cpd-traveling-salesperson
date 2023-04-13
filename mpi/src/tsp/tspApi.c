#include "tspApi.h"
#include "tspNode.h"
#include "tspSolver.h"
#include <stddef.h>

MPI_Datatype tspApiSolutionDatatype() {
    MPI_Datatype newType;

    const int nBlocks = 4;
    const int blockLengths[] = {1, 1, 1, MAX_CITIES};
    const MPI_Datatype blockTypes[] = {MPI_C_BOOL, MPI_DOUBLE, MPI_DOUBLE, MPI_CHAR};

    MPI_Aint blockDisplacements[nBlocks];
    blockDisplacements[0] = (MPI_Aint)offsetof(tspSolution_t, hasSolution);
    blockDisplacements[1] = (MPI_Aint)offsetof(tspSolution_t, cost);
    blockDisplacements[2] = (MPI_Aint)offsetof(tspSolution_t, priority);
    blockDisplacements[3] = (MPI_Aint)offsetof(tspSolution_t, tour);

    MPI_Type_create_struct(nBlocks, blockLengths, blockDisplacements, blockTypes, &newType);
    MPI_Type_commit(&newType);
    return newType;
}

MPI_Datatype tspApiNodeDatatype() {
    MPI_Datatype newType;

    const int nBlocks = 6;
    const int blockLengths[] = {1, 1, 1, 1, MAX_CITIES, 1};
    const MPI_Datatype blockTypes[] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_CHAR, MPI_UNSIGNED_LONG_LONG};

    MPI_Aint blockDisplacements[nBlocks];
    blockDisplacements[0] = (MPI_Aint)offsetof(tspNode_t, cost);
    blockDisplacements[1] = (MPI_Aint)offsetof(tspNode_t, lb);
    blockDisplacements[2] = (MPI_Aint)offsetof(tspNode_t, priority);
    blockDisplacements[3] = (MPI_Aint)offsetof(tspNode_t, length);
    blockDisplacements[4] = (MPI_Aint)offsetof(tspNode_t, tour);
    blockDisplacements[5] = (MPI_Aint)offsetof(tspNode_t, visited);

    MPI_Type_create_struct(nBlocks, blockLengths, blockDisplacements, blockTypes, &newType);
    MPI_Type_commit(&newType);
    return newType;
}

tspApi_t* tspApiCreate() {
    tspApi_t* api = (tspApi_t*)malloc(sizeof(tspApi_t));
    api->procId = -1;
    api->nProcs = 0;
    return api;
}

void tspApiDestroy(tspApi_t* api) { free(api); }

void tspApiInit(tspApi_t* api) {
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &api->procId);
    MPI_Comm_size(MPI_COMM_WORLD, &api->nProcs);
    api->procType = (api->procId == 0 ? PROCTYPE_MASTER : PROCTYPE_TASK);
    api->solution_t = tspApiSolutionDatatype();
    api->node_t = tspApiNodeDatatype();
}

void tspApiTerminate(tspApi_t* api) {
    api->procId = -1;
    api->nProcs = -1;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}