#include "tspApi.h"
#include "tspNode.h"
#include "tspSolver.h"
#include <stddef.h>

MPI_Datatype tspApiProblemDatatype() {
    MPI_Datatype newType;

    const int nBlocks = 3;
    const int blockLengths[] = {1, MAX_CITIES * MAX_CITIES, MAX_CITIES * TSP_TOTAL_MIN_COSTS};
    const MPI_Datatype blockTypes[] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE};

    MPI_Aint blockDisplacements[nBlocks];
    blockDisplacements[0] = (MPI_Aint)offsetof(tsp_t, nCities);
    blockDisplacements[1] = (MPI_Aint)offsetof(tsp_t, roadCosts);
    blockDisplacements[2] = (MPI_Aint)offsetof(tsp_t, minCosts);

    MPI_Type_create_struct(nBlocks, blockLengths, blockDisplacements, blockTypes, &newType);
    MPI_Type_commit(&newType);
    return newType;
}

// MPI_Datatype tspApiSolutionDatatype() {
//     MPI_Datatype newType;

//     const int nBlocks = 4;
//     const int blockLengths[] = {1, 1, 1, MAX_CITIES};
//     const MPI_Datatype blockTypes[] = {MPI_C_BOOL, MPI_DOUBLE, MPI_DOUBLE, MPI_CHAR};

//     MPI_Aint blockDisplacements[nBlocks];
//     blockDisplacements[0] = (MPI_Aint)offsetof(tspSolution_t, hasSolution);
//     blockDisplacements[1] = (MPI_Aint)offsetof(tspSolution_t, cost);
//     blockDisplacements[2] = (MPI_Aint)offsetof(tspSolution_t, priority);
//     blockDisplacements[3] = (MPI_Aint)offsetof(tspSolution_t, tour);

//     MPI_Type_create_struct(nBlocks, blockLengths, blockDisplacements, blockTypes, &newType);
//     MPI_Type_commit(&newType);
//     return newType;
// }

MPI_Datatype tspApiControlDatatype() {
    MPI_Datatype newType;

    const int nNodes = MAX_CONTROL_NODES;
    const int nBlocks = 13;
    const int blockLengths[] = {1, 1, 1, 1, 1, MAX_CITIES, 1, nNodes, nNodes, nNodes, nNodes, nNodes * MAX_CITIES, nNodes};

    MPI_Datatype blockTypes[nBlocks];
    blockTypes[0] = MPI_C_BOOL;
    blockTypes[1] = MPI_DOUBLE;
    blockTypes[2] = MPI_C_BOOL;
    blockTypes[3] = MPI_DOUBLE;
    blockTypes[4] = MPI_DOUBLE;
    blockTypes[5] = MPI_CHAR;
    blockTypes[6] = MPI_INT;
    blockTypes[7] = MPI_DOUBLE;
    blockTypes[8] = MPI_DOUBLE;
    blockTypes[9] = MPI_DOUBLE;
    blockTypes[10] = MPI_INT;
    blockTypes[11] = MPI_CHAR;
    blockTypes[12] = MPI_UNSIGNED_LONG_LONG;

    MPI_Aint blockDisplacements[nBlocks];
    blockDisplacements[0] = (MPI_Aint)offsetof(tspApiControl_t, running);
    blockDisplacements[1] = (MPI_Aint)offsetof(tspApiControl_t, delay);
    blockDisplacements[2] = (MPI_Aint)offsetof(tspApiControl_t, hasSolution);
    blockDisplacements[3] = (MPI_Aint)offsetof(tspApiControl_t, solutionCost);
    blockDisplacements[4] = (MPI_Aint)offsetof(tspApiControl_t, solutionPriority);
    blockDisplacements[5] = (MPI_Aint)offsetof(tspApiControl_t, solutionTour);
    blockDisplacements[6] = (MPI_Aint)offsetof(tspApiControl_t, nNodes);
    blockDisplacements[7] = (MPI_Aint)offsetof(tspApiControl_t, costs);
    blockDisplacements[8] = (MPI_Aint)offsetof(tspApiControl_t, lbs);
    blockDisplacements[9] = (MPI_Aint)offsetof(tspApiControl_t, priorities);
    blockDisplacements[10] = (MPI_Aint)offsetof(tspApiControl_t, lengths);
    blockDisplacements[11] = (MPI_Aint)offsetof(tspApiControl_t, tours);
    blockDisplacements[12] = (MPI_Aint)offsetof(tspApiControl_t, visited);

    MPI_Type_create_struct(nBlocks, blockLengths, blockDisplacements, blockTypes, &newType);
    MPI_Type_commit(&newType);
    return newType;
}

tspApi_t* tspApiInit() {
    tspApi_t* api = (tspApi_t*)malloc(sizeof(tspApi_t));
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &api->procId);
    MPI_Comm_size(MPI_COMM_WORLD, &api->nProcs);
    api->procType = (api->procId == 0 ? PROCTYPE_MASTER : PROCTYPE_TASK);
    api->problem_t = tspApiProblemDatatype();
    api->control_t = tspApiControlDatatype();
    return api;
}

void tspApiTerminate(tspApi_t* api) {
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    free(api);
}
