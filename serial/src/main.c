#include "include.h"
#include "tsp/tsp.h"
#include "tsp/tspSolver.h"
#include "utils/file.h"

tsp_t parseInput(const char* inPath) {
    FILE* inputFile = openFile(inPath, "r");
    size_t nCities, nRoads;
    fscanf(inputFile, "%lu %lu\n", &nCities, &nRoads);
    tsp_t tsp = tspCreate(nCities, nRoads);

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

void printSolution(const tsp_t* tsp, const tspSolution_t* solution) {
    if (solution->hasSolution) {
        printf("%.1f\n", solution->cost);
        for (size_t i = 0; i < tsp->nCities; i++)
            printf("%ld ", solution->tour[i]);
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
    double maxTourCost = atoi(argv[2]);
    LOG("inPath = %s", inPath);
    LOG("maxTourCost = %f", maxTourCost);
    tsp_t tsp = parseInput(inPath);
    DEBUG(tspPrint(&tsp));

    double execTime = -omp_get_wtime();
    tspSolution_t solution = tspSolve(&tsp, maxTourCost);
    execTime += omp_get_wtime();

    fprintf(stderr, "%.1fs\n", execTime);
    printSolution(&tsp, &solution);

    tspSolutionDestroy(&solution);
    tspDestroy(&tsp);
    return 0;
}