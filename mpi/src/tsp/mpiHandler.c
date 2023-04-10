#include <mpi.h>
#include "mpiHandler.h"




MPI_Datatype mpiSolutionDataType() {
    // https://stackoverflow.com/questions/20040663/how-to-create-new-type-in-mpi
    MPI_Datatype new_type;

    int count = 3;
    int blocklens[] = {1, 1, MAX_CITIES};

    MPI_Aint indices[3];
    indices[0] = (MPI_Aint)offsetof(tspSolution_t, hasSolution);
    indices[1] = (MPI_Aint)offsetof(tspSolution_t, cost);
    indices[2] = (MPI_Aint)offsetof(tspSolution_t, tour);

    MPI_Datatype old_types[] = {MPI_C_BOOL, MPI_DOUBLE, MPI_CHAR};

    MPI_Type_create_struct(count, blocklens, indices, old_types, &new_type);
    MPI_Type_commit(&new_type);

    return new_type;
}

MPI_Datatype mpiNodeDataType() {
    MPI_Datatype new_type;

    int count = 6;
    int blocklens[] = {1, 1, 1, 1, MAX_CITIES, 1};

    MPI_Aint indices[7];
    indices[0] = (MPI_Aint)offsetof(Node_t, cost);
    indices[1] = (MPI_Aint)offsetof(Node_t, lb);
    indices[2] = (MPI_Aint)offsetof(Node_t, priority);
    indices[3] = (MPI_Aint)offsetof(Node_t, length);
    indices[4] = (MPI_Aint)offsetof(Node_t, tour);
    indices[5] = (MPI_Aint)offsetof(Node_t, visited);

    MPI_Datatype old_types[] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_CHAR, MPI_UNSIGNED_LONG_LONG};

    MPI_Type_create_struct(count, blocklens, indices, old_types, &new_type);
    MPI_Type_commit(&new_type);

    return new_type;
}