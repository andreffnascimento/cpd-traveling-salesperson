#ifndef __TSP_MPI_HANDLER__
#define __TSP_MPI_HANDLER__

#include "tspSolver.h"
#include "node.h"
#include <mpi.h>

#define NODE_TAG 100
#define SOLUTION_TAG 101
#define PROCESSING_STATUS_TAG 102
#define TERMINATED_TAG 103
#define INIT_TAG 104

/*---------------------------------------------*
---------------- Sync Mechanisms --------------*
-----------------------------------------------*/
void sendNode(Node_t* node, int to);
void recvNode(Node_t* node, int from);


/*---------------------------------------------*
--------------- Async Mechanisms --------------*
-----------------------------------------------*/
void sendAsyncSolution(tspSolution_t* solution, int to);
void recvAsyncSolution(tspSolution_t* solution, int from);
void sendAsyncNode(Node_t* node, int to);
void recvAsyncNode(Node_t* node, int from);

/*---------------------------------------------*
-------------- Control Mechanisms -------------*
-----------------------------------------------*/
bool hasMessageToReceive(int source, int tag);

/*---------------------------------------------*
----------------- MPI DataTypes ---------------*
-----------------------------------------------*/

MPI_Datatype mpiSolutionDataType();
MPI_Datatype mpiNodeDataType();


#endif //__TSP_MPI_HANDLER__