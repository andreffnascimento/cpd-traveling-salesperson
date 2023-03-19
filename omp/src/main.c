#include "include.h"
#include "tsp/tsp.h"
#include "utils/file.h"

tsp_t parseInput(const char* inPath, double bestTourCost) {
    FILE* inputFile = openFile(inPath, "r");
    size_t nCities, nRoads;
    fscanf(inputFile, "%lu %lu\n", &nCities, &nRoads);
    tsp_t tsp = tspCreate(nCities, nRoads, bestTourCost);

    for (size_t i = 0; i < tsp.nRoads; i++) {
        int cityA, cityB;
        double cost;
        fscanf(inputFile, "%d %d %le\n", &cityA, &cityB, &cost);
        tsp.roadCosts[cityA][cityB] = cost;
        tsp.roadCosts[cityB][cityA] = cost;
    }

    closeFile(inputFile);
    return tsp;
}

void printSolution(const tspNode_t* solution) {
    if (solution != NULL) {
        printf("%.1f\n", solution->cost);
        for (size_t i = 0; i < solution->length; i++) {
            if (i == solution->length - 1)
                printf("%ld", solution->tour[i]);
            else
                printf("%ld ", solution->tour[i]);
        }
        printf("\n");
    } else {
        printf("NO SOLUTION\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./tsp <cities_file> <max_value>\n");
        exit(1);
    }

    omp_set_num_threads(4);
    const char* inPath = argv[1];
    double bestTourCost = atoi(argv[2]);
    LOG("inPath = %s", inPath);
    LOG("bestTourCost = %f", bestTourCost);
    tsp_t tsp = parseInput(inPath, bestTourCost);
    DEBUG(tspPrint(&tsp));

    double execTime = -omp_get_wtime();
    tspSolve(&tsp);
    execTime += omp_get_wtime();

    fprintf(stderr, "%.1fs\n", execTime);
    printSolution(tsp.solution);

    if (tsp.solution != NULL)
        tspNodeDestroy(tsp.solution);
    tspDestroy(&tsp);
    return 0;
}