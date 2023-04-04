#ifndef __TSP__TSP_CONTAINER_H__
#define __TSP__TSP_CONTAINER_H__

#include "include.h"
#include "node.h"

typedef struct ContainerEntry ContainerEntry_t;
typedef struct Container Container_t;


Container_t* containerCreate();
ContainerEntry_t* containerGetEntry(Container_t* container);
Node_t* containerGetNode(ContainerEntry_t* entry);

void containerDestroy(Container_t* container);
void containerRemoveEntry(Container_t* container, ContainerEntry_t* entry);

#endif // __TSP__TSP_CONTAINER_H__
