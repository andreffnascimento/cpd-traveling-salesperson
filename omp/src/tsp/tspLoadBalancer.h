#ifndef __TSP__TSP_LOAD_BALANCER_H__
#define __TSP__TSP_LOAD_BALANCER_H__

#include "include.h"
#include "tspNode.h"

typedef struct _tspLoadBalancer tspLoadBalancer_t;

tspLoadBalancer_t* tspLoadBalancerCreate(int nThreads);
void tspLoadBalancerDestroy(tspLoadBalancer_t* tspLoadBalancer);

tspNode_t* tspLoadBalancerPop(tspLoadBalancer_t* tspLoadBalancer, double* solutionPriority);
tspNode_t* tspLoadBalancerPush(tspLoadBalancer_t* tspLoadBalancer, tspNode_t* tspNode);

#endif // __TSP__TSP_LOAD_BALANCER_H__
