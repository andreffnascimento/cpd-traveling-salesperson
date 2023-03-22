#ifndef __TSP_THREADED_QUEUE_H__
#define __TSP_THREADED_QUEUE_H__

#include "include.h"
#include "utils/queue.h"

/***********************************************************
---------------------- THREADED QUEUE ----------------------
***********************************************************/

typedef struct _tspSolver tspSolver_t;

tspSolver_t* tspSolverCreate(size_t nThreads, int (*cmpFun)(void*, void*));
void tspSolverDestroy(tspSolver_t* tspSolver, void (*delFun)(void*));

bool tspSolverRunning(const tspSolver_t* tspSolver);
void tspSolverStartThread(tspSolver_t* tspSolver, size_t threadNum);
void tspSolverStopThread(tspSolver_t* tspSolver, size_t threadNum);

void* tspSolverIndexPop(tspSolver_t* tspSolver, size_t threadNum);
void tspSolverIndexPush(tspSolver_t* tspSolver, size_t threadNum, void* element);
void tspSolverSeqPush(tspSolver_t* tspSolver, void* element);

#endif // __TSP_THREADED_QUEUE_H__
