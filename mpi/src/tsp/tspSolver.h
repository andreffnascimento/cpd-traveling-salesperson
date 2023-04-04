#ifndef __TSP__TSP_SOLVER_H__
#define __TSP__TSP_SOLVER_H__

#include "include.h"
#include "tsp.h"


typedef struct {
    bool hasSolution;
    double cost;
    char tour[MAX_CITIES];
} tspSolution_t;

tspSolution_t* tspSolutionCreate(double maxTourCost);
void tspSolutionDestroy(tspSolution_t* tspSolution);

tspSolution_t* tspSolve(const tsp_t* tsp, double maxTourCost);

#endif // __TSP__TSP_SOLVER_H__
