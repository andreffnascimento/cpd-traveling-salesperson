#include "include.h"
#include "tsp/tsp.h"
#include "utils/file.h"

tsp_t parseInput(const char* inPath, int maxValue) {
    FILE* inputFile = openFile(inPath, "r");
    int nCities, nRoads;
    fscanf(inputFile, "%d %d\n", &nCities, &nRoads);
    tsp_t tsp = tspCreate(nCities, nRoads);

    for (int i = 0; i < tsp.nRoads; i++) {
        tspRoad_t road;
        fscanf(inputFile, "%d %d %d\n", &road.cityA, &road.cityB, &road.cost);
        tsp.roads[i] = road;
    }

    fclose(inputFile);
    return tsp;
}

void printSolution(const tsp_t* tsp) {
    const tspSolution_t* solution = &tsp->solution;
    if (solution->hasSolution) {
        printf("%f\n", solution->cost);
        for (int i = 0; i < tsp->nCities; i++)
            printf("%d ", solution->cities[i]);
        printf("0\n");
    } else {
        printf("NO SOLUTION\n");
    }
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

    tsp_t tsp = parseInput(inPath, maxValue);
    DEBUG(tspPrint(&tsp));
    tspSolve(&tsp);
    printSolution(&tsp);
    tspFree(&tsp);

    return 0;
}