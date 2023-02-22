#include <stdio.h>
#include <stdlib.h>

#include "tsp/tsp.h"
#include "utils/file.h"
#include "utils/log.h"


tsp_t parseTSP(const char* inPath, int maxValue) {
    FILE* inputFile = openFile(inPath, "r");
    tsp_t tsp;
    fscanf(inputFile, "%d %d\n", &tsp.nCities, &tsp.nRoads);
    tsp.roads = malloc(tsp.nRoads * sizeof(tspRoad_t));

    for (int i = 0; i < tsp.nRoads; i++) {
        tspRoad_t road;
        fscanf(inputFile, "%d %d %d\n", &road.cityA, &road.cityB, &road.cost);
        tsp.roads[i] = road;
    }

    fclose(inputFile);
    return tsp;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./tsp <cities_file> <max_value>\n");
        exit(1);
    }

    const char* inPath = argv[1];
    int maxValue = atoi(argv[2]);
    LOG("inPath = %s", inPath);
    LOG("maxValue = %d", maxValue);

    tsp_t tsp = parseTSP(inPath, maxValue);
    DEBUG(tspPrint(&tsp));
    tspFree(&tsp);

    return 0;
}