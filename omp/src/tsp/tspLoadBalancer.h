#ifndef __TSP__TSP_LOAD_BALANCER_H__
#define __TSP__TSP_LOAD_BALANCER_H__

#include "include.h"
#include "tspContainer.h"

typedef struct _tspLoadBalancer tspLoadBalancer_t;

tspLoadBalancer_t* tspLoadBalancerCreate(int nThreads, int (*cmpFun)(void*, void*));
void tspLoadBalancerDestroy(tspLoadBalancer_t* tspLoadBalancer);

tspContainer_t* tspLoadBalancerGetContainer(tspLoadBalancer_t* tspLoadBalancer);
void tspLoadBalancerDestroyEntry(tspLoadBalancer_t* tspLoadBalancer, tspContainerEntry_t* entry);

tspContainerEntry_t* tspLoadBalancerPop(tspLoadBalancer_t* tspLoadBalancer, double branchVal);
void tspLoadBalancerPush(tspLoadBalancer_t* tspLoadBalancer, const tspNode_t* parent, double cost, double lb,
                         int cityNumber);

#endif // __TSP__TSP_LOAD_BALANCER_H__
