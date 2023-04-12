#include "include.h"
#include "tsp/tspSolver.h"
#include <omp.h>

FILE* openFile(const char* path, const char* mode) {
    FILE* file = fopen(path, mode);
    if (file == NULL) {
        printf("Unable to open the file: %s\n", path);
        exit(1);
    }
    return file;
}

tsp_t parseInput(const char* inPath) {
    FILE* inputFile = openFile(inPath, "r");
    size_t nCities, nRoads;
    fscanf(inputFile, "%lu %lu\n", &nCities, &nRoads);
    tsp_t tsp = tspCreate(nCities, nRoads);

    for (int i = 0; i < tsp.nRoads; i++) {
        int cityA, cityB;
        double cost;
        fscanf(inputFile, "%d %d %le\n", &cityA, &cityB, &cost);
        tsp.roadCosts[cityA][cityB] = cost;
        tsp.roadCosts[cityB][cityA] = cost;
    }

    fclose(inputFile);
    tspInitializeMinCosts(&tsp);
    return tsp;
}

void printSolution(const tsp_t* tsp, const tspSolution_t* solution) {
    if (solution->hasSolution) {
        printf("%.1f\n", solution->cost);
        for (int i = 0; i < tsp->nCities; i++)
            printf("%d ", solution->tour[i]);
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

    int numThreads;
    #pragma omp parallel
    #pragma omp single
    numThreads = omp_get_num_threads();

    double execTime = -omp_get_wtime();
    tspSolution_t* solution = tspSolve(&tsp, maxTourCost, numThreads);
    execTime += omp_get_wtime();

    fprintf(stderr, "%.1fs\n", execTime);
    printSolution(&tsp, solution);

    tspSolutionDestroy(solution);
    tspDestroy(&tsp);
    return 0;
}