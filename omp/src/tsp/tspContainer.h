#ifndef __TSP__TSP_CONTAINER_H__
#define __TSP__TSP_CONTAINER_H__

#include "include.h"
#include "tspNode.h"

typedef struct tspContainerEntry tspContainerEntry_t;
typedef struct tspContainer tspContainer_t;

tspContainer_t* tspContainerCreate();
void tspContainerDestroy(tspContainer_t* container);
tspContainerEntry_t* tspContainerGetEntry(tspContainer_t* container);
void tspContainerRemoveEntry(tspContainer_t* container, tspContainerEntry_t* entry);
tspNode_t* tspContainerEntryVal(tspContainerEntry_t* entry);

#endif // __TSP__TSP_CONTAINER_H__
