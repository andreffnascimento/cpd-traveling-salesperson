#ifndef __TSP_TSP_H__
#define __TSP_TSP_H__

#include "include.h"

//Prototypes
typedef struct tspNode tspNode_t;

/*----------------------------------------------------------
_____________________________tsp___________________________
----------------------------------------------------------*/

typedef struct {
    bool hasSolution;
    double cost;
    tspNode_t *bestTour;
    int maxValue;
} tspSolution_t;

typedef struct {
    size_t nCities;
    size_t nRoads;
    double** roads;
    priorityQueue_t trashQueue;
    priorityQueue_t queue;
    tspSolution_t solution;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads);
void tspDelete(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
void tspSolve(tsp_t* tsp);

/*----------------------------------------------------------
__________________________tsp Node ________________________
----------------------------------------------------------*/

typedef struct tspNode {
    struct tspNode *tour;
    double cost;
    double lb;
    int length;
    int currentCity;

} tspNode_t;

tspNode_t* tspCreateNode(tspNode_t* tour, double cost, double lb, int length, int currentCity);
void tspDeleteNode(tspNode_t *node);

#endif // __TSP_TSP_H__