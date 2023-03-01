#include "tsp.h"
#include <math.h>
#include "utils/queue.h"

tsp_t tspCreate(size_t nCities, size_t nRoads)
{
    tsp_t tsp;
    tsp.nCities = nCities;
    tsp.nRoads = nRoads;
    // tsp.roads = malloc(tsp.nRoads * sizeof(tspRoad_t));

    // Initialization of the Roads to Null
    tsp.roads = (double **)malloc(tsp.nRoads * sizeof(double *));
    for (size_t i = 0; i < tsp.nCities; i++)
    {
        tsp.roads[i] = (double *)malloc(tsp.nRoads * sizeof(double));
        for (size_t j = 0; j < tsp.nCities; j++)
        {
            //Assuming we don't have negative costs or use INFINITY
            tsp.roads[i][j] = -1; // Init value for every road in math.h
        }
    }

    tsp.solution.hasSolution = false;
    tsp.solution.cost = INFINITY;
    tsp.solution.bestTour = NULL;
    // ISSUE: excessive amount of allocation of memory with the possible result
    // tsp.solution.cities = malloc(tsp.nCities * sizeof(int));
    return tsp;
}

void tspDelete(tsp_t *tsp) {
    free(tsp->solution.bestTour);
    free(tsp->roads);
    tsp->solution.bestTour = NULL;
    tsp->roads = NULL;
}

void tspPrint(const tsp_t *tsp) {
    printf("TSP{ nCities = %lu, nRoads = %lu }\n", tsp->nCities, tsp->nRoads);
    for (size_t i = 0; i < tsp->nCities; i++)
    {
        for (size_t j = 0; j < tsp->nCities; j++)
        {
            printf(" - Road { %ld <-> %ld (cost = %f) }\n", i, j, tsp->roads[i][j]);
        }
    }
}

tspNode_t *tspCreateNode(tspNode_t *tour, double cost, double lb, int length, int currentCity) {

    tspNode_t *node = malloc(sizeof(tspNode_t));
    node->tour = tour; // should be a queue
    node->cost = cost;
    node->lb = lb;
    node->length = length;
    node->currentCity = currentCity;
    return node;
}

void tspDeleteNode(tspNode_t *node) {
    free(node);
}


int compFun(tspNode_t *n1, tspNode_t *n2) {
    if (n1->lb <= n2->lb) return 0;
    return 1;
}

bool isCityInTour(tspNode_t *node, int cityNumber) {
    //Optimization:
    // I think in the future if needed this could be 
    // optimized with an HashTable or smth similar
    tspNode_t *analyzer = node;
    while (true) {
        if (analyzer == NULL) return false;
        else if (analyzer->currentCity == cityNumber) return true;
        else analyzer = analyzer->tour;
    }
}

bool isNeighbor(const tsp_t *tsp, int cityA, int cityB) {
    return (tsp->roads[cityA][cityB] != -1.0) && (cityA != cityB);
}


double initial_lb(const tsp_t *tsp) {
    // For city 0
    // Assumption: we always have two different paths for the same city
    register double min1, min2, temp, sum;
    sum = 0;

    DEBUG("Init inital_lb");

    for (size_t i = 0; i < tsp->nCities; i++) {
        min1 = min2 = INFINITY;

        for (size_t j = 0; j < tsp->nCities; j++) {

            temp = tsp->roads[i][j];
            if (isNeighbor(tsp,i,j)) {

                if (temp < min1) {
                    min2 = min1;
                    min1 = temp;
                    // if (min2 == INFINITY){
                    //     min2 = min1;
                    //     DEBUG("Entered Here");
                    // } 
                }
                else if (temp < min2) min2 = temp;
            }
            else continue;
        }
        printf("i=%d min1=%le min2=%le\n",i,min1,min2);
        sum += min1 + min2;
    }

    printf("[DEBUG] Initial LB %le\n", sum/2);
    return sum / 2;
}


double calculate_lb(const tsp_t *tsp, tspNode_t *node, int cityNumber) {
    // double newLb = node->lb + tsp->roads[node->currentCity][cityNumber]
    // DEBUG("Calculate_lb");
    // printf("From=%d To=%d\n",node->currentCity,cityNumber);

    double min1_from, min2_from, min1_t, min2_t, cost_f_t, cf, ct;
    register int cityFrom, cityTo;

    cityFrom = node->currentCity;
    cityTo = cityNumber;

    cost_f_t = tsp->roads[node->currentCity][cityNumber];

    min1_from = min2_from = INFINITY;
    min1_t = min2_t = INFINITY;

    for (size_t i = 0; i < tsp->nCities; i++) {
        //min1_from, min2_from calculation
        if (isNeighbor(tsp, i, cityFrom)) {
            if (tsp->roads[i][cityFrom] < min1_from) {
                min2_from = min1_from;
                min1_from = tsp->roads[i][cityFrom];
                // if (min2_from == INFINITY) min2_from = tsp->roads[i][cityFrom];
            }
            else if (tsp->roads[i][cityFrom] < min2_from) min2_from = tsp->roads[i][cityFrom];
        }
        //min1_t, min2_t calculation
        if (isNeighbor(tsp, i, cityTo)) {
            if (tsp->roads[i][cityTo] < min1_t) {
                min2_t = min1_t;
                min1_t = tsp->roads[i][cityTo];
                // if (min2_from == INFINITY) min2_from = tsp->roads[i][cityTo];
            }
            else if (tsp->roads[i][cityTo] < min2_t) min2_t = tsp->roads[i][cityTo];
        }
    }

    //cf calculation
    if (cost_f_t >= min2_from) cf = min2_from;
    else cf = min1_from;

    //ct calculation
    if (cost_f_t >= min2_t) ct = min2_t;
    else ct = min1_t;

    // printf("min1_from %le min2_from %le min1_t %lf min2_t %lf\n");
    // printf("calculate_lb result %le\n", (node->lb + cost_f_t - (cf + ct)/2) );
    return node->lb + cost_f_t - (cf + ct)/2;

}


void tspSolve(tsp_t *tsp) {
    DEBUG("Inside tspSolve");
    // Init Process
    priorityQueue_t queue = queueCreate(&compFun);
    DEBUG("Queue Created");
    tspNode_t *startNode = tspCreateNode(NULL, 0, initial_lb(tsp), 1, 0);
    DEBUG("tspCreatedNode");
    queuePush(&queue, startNode);
    DEBUG("Queue Push 0");

    tspNode_t *node;
    while (true) {
        //DOUBT: is needed or since it is null is detected
        
        node = queuePop(&queue);

        if (node == NULL) return;

        printf("From: %d Node %d\n", (node->tour ? node->tour->currentCity : -1), node->currentCity);

        
        // DEBUG("Node not NULL");

        if (node->lb >= tsp->solution.maxValue) {
            //tsp->solution.hasSolution = true;
            //tsp->solution.bestTour = node;
            //tsp->solution.cost = node->cost;
            // tsp->solution.cost = node->cost;
            printf("[+] Elements in queue are not important\n");
            return;
        }

        if (node->length == tsp->nCities) {
            DEBUG("All citites visited");
            // we already visited all the cities
            //if ((node->cost + tsp->roads[node->currentCity][0]) < tsp->solution.maxValue)  {
            if ( ((node->cost + tsp->roads[node->currentCity][0]) < tsp->solution.maxValue) && ((node->cost + tsp->roads[node->currentCity][0]) < tsp->solution.cost) ) {
                tspNode_t *lastNode = tspCreateNode(node, node->cost + tsp->roads[node->currentCity][0], calculate_lb(tsp, node, 0), node->length, 0);
                tsp->solution.hasSolution = true;
                tsp->solution.bestTour = lastNode;
                tsp->solution.cost = lastNode->cost;
            }
        }
        else {
            DEBUG("Visiting Nodes");
            double lb;
            for (size_t cityNumber = 0; cityNumber < tsp->nCities; cityNumber++) {   
                //ASSUMPTION: cities have increased sequential number
                if (isNeighbor(tsp, node->currentCity, cityNumber) && !isCityInTour(node, cityNumber)) {
                    lb = calculate_lb(tsp, node, cityNumber);
                    
                    if (lb > tsp->solution.maxValue) continue;

                    tspNode_t *nextNode = tspCreateNode(node, node->cost + tsp->roads[node->currentCity][cityNumber], lb, node->length + 1, cityNumber);
                    queuePush(&queue, nextNode);
                }
            }
            
        }
    }
    DEBUG("[+] Finished tspSolve");
}
