#ifndef __TSP_TSP_H__
#define __TSP_TSP_H__

#include "include.h"

#define NONEXISTENT_ROAD_VALUE -1

/***********************************************************
-------------------------- TSP -----------------------------
***********************************************************/

typedef struct tspSmallNode {
    struct tspSmallNode* parent;
    int currentCity;
} tspSmallNode_t;

tspSmallNode_t* tspCreateSmallNode(tspSmallNode_t* parent, int currentCity);
void tspDestroySmallNode(tspSmallNode_t* node);

typedef struct tspNode {
    tspSmallNode_t *parent;
    double cost;
    double lb;
    int length;
    int currentCity;

} tspNode_t;

tspNode_t* tspCreateNode(tspSmallNode_t* parent, double cost, double lb, int length, int currentCity);
void tspDestroyNode(tspNode_t* node);


typedef struct {
    bool hasSolution;
    double cost;
    tspSmallNode_t* bestTour;
} tspSolution_t;

typedef struct {
    size_t nCities;
    size_t nRoads;
    double** roadCosts;
    priorityQueue_t trashQueue;
    priorityQueue_t queue;
    tspSolution_t solution;
} tsp_t;

tsp_t tspCreate(size_t nCities, size_t nRoads);
void tspDestroy(tsp_t* tsp);
void tspPrint(const tsp_t* tsp);
void tspSolve(tsp_t* tsp, int maxValue);

#endif // __TSP_TSP_H__