#include "mpiHandler.h"
#include <mpi.h>


#define NODE_TAG 100
#define SOLUTION_TAG 101

void sendAsyncSolution(tspSolution_t* solution, int to) {
    MPI_Request requests[3];
    MPI_Isend(&solution->cost, 1, MPI_DOUBLE, to, SOLUTION_TAG, MPI_COMM_WORLD, &requests[0]);
    MPI_Isend(&solution->hasSolution, 1, MPI_CXX_BOOL, to, SOLUTION_TAG, MPI_COMM_WORLD, &requests[1]);
    MPI_Isend(&solution->tour, MAX_CITIES, MPI_CHAR, to, SOLUTION_TAG, MPI_COMM_WORLD, &requests[2]);
}

void recvAsyncSolution(tspSolution_t* solution, int to) {
    MPI_Request requests[3];
    MPI_Irecv(&solution->cost, 1, MPI_DOUBLE, to, SOLUTION_TAG, MPI_COMM_WORLD, &requests[0]);
    MPI_Irecv(&solution->hasSolution, 1, MPI_CXX_BOOL, to, SOLUTION_TAG, MPI_COMM_WORLD, &requests[1]);
    MPI_Irecv(&solution->tour, MAX_CITIES, MPI_CHAR, to, SOLUTION_TAG, MPI_COMM_WORLD, &requests[2]);
}
bool hasMessageToReceive(int source, int tag) {
    MPI_Status status;
    int flag = 0;
    MPI_Iprobe(source, tag, MPI_COMM_WORLD, &flag, &status);
    return flag > 0;
}

void sendNode(Node_t* node, int to) {
    MPI_Send(&node->cost, 1, MPI_DOUBLE, to, NODE_TAG, MPI_COMM_WORLD);    
    MPI_Send(&node->lb, 1, MPI_DOUBLE, to, NODE_TAG, MPI_COMM_WORLD);    
    MPI_Send(&node->priority, 1, MPI_DOUBLE, to, NODE_TAG, MPI_COMM_WORLD);    
    MPI_Send(&node->length, 1, MPI_INT, to, NODE_TAG, MPI_COMM_WORLD);    
    MPI_Send(&node->tour, MAX_CITIES, MPI_CHAR, to, NODE_TAG, MPI_COMM_WORLD);    
    MPI_Send(&node->visited, 1, MPI_LONG_LONG, to, NODE_TAG, MPI_COMM_WORLD);    
}

void recvNode(Node_t* node, int from) {
    MPI_Status status[6];
    MPI_Recv(&node->cost, 1, MPI_DOUBLE, from, NODE_TAG, MPI_COMM_WORLD, &status[0]);    
    MPI_Recv(&node->lb, 1, MPI_DOUBLE, from, NODE_TAG, MPI_COMM_WORLD, &status[1]);    
    MPI_Recv(&node->priority, 1, MPI_DOUBLE, from, NODE_TAG, MPI_COMM_WORLD, &status[2]);    
    MPI_Recv(&node->length, 1, MPI_INT, from, NODE_TAG, MPI_COMM_WORLD, &status[3]);    
    MPI_Recv(&node->tour, MAX_CITIES, MPI_CHAR, from, NODE_TAG, MPI_COMM_WORLD, &status[4]);    
    MPI_Recv(&node->visited, 1, MPI_LONG_LONG, from, NODE_TAG, MPI_COMM_WORLD, &status[5]);    
}

void sendAsyncNode(Node_t* node, int to) {
    MPI_Request requests[6];
    MPI_Isend(&node->cost, 1, MPI_DOUBLE, to, NODE_TAG, MPI_COMM_WORLD, &requests[0]);    
    MPI_Isend(&node->lb, 1, MPI_DOUBLE, to, NODE_TAG, MPI_COMM_WORLD, &requests[1]);    
    MPI_Isend(&node->priority, 1, MPI_DOUBLE, to, NODE_TAG, MPI_COMM_WORLD, &requests[2]);    
    MPI_Isend(&node->length, 1, MPI_INT, to, NODE_TAG, MPI_COMM_WORLD, &requests[3]);    
    MPI_Isend(&node->tour, MAX_CITIES, MPI_CHAR, to, NODE_TAG, MPI_COMM_WORLD, &requests[4]);    
    MPI_Isend(&node->visited, 1, MPI_LONG_LONG, to, NODE_TAG, MPI_COMM_WORLD, &requests[5]);    
}

void recvAsyncNode(Node_t* node, int from) {
    MPI_Request requests[6];
    MPI_Irecv(&node->cost, 1, MPI_DOUBLE, from, NODE_TAG, MPI_COMM_WORLD, &requests[0]);    
    MPI_Irecv(&node->lb, 1, MPI_DOUBLE, from, NODE_TAG, MPI_COMM_WORLD, &requests[1]);    
    MPI_Irecv(&node->priority, 1, MPI_DOUBLE, from, NODE_TAG, MPI_COMM_WORLD, &requests[2]);    
    MPI_Irecv(&node->length, 1, MPI_INT, from, NODE_TAG, MPI_COMM_WORLD, &requests[3]);    
    MPI_Irecv(&node->tour, MAX_CITIES, MPI_CHAR, from, NODE_TAG, MPI_COMM_WORLD, &requests[4]);    
    MPI_Irecv(&node->visited, 1, MPI_LONG_LONG, from, NODE_TAG, MPI_COMM_WORLD, &requests[5]);    
}