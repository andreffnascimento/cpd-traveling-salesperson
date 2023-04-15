#include "include.h"
#include "tsp/tspSolver.h"
#include <omp.h>

FILE* openFile(const char* path, const char* mode) {
    FILE* file = fopen(path, mode);
    if (file == NULL) {
        printf("Unable to open the file: %s\n", path);
        exit(EXIT_FAILURE);
    }
    return file;
}

tsp_t* parseInput(const char* inPath) {
    FILE* inputFile = openFile(inPath, "r");
    int nRoads;
    tsp_t* tsp = tspCreate();
    fscanf(inputFile, "%d %d\n", &tsp->nCities, &nRoads);

    for (int i = 0; i < nRoads; i++) {
        int cityA, cityB;
        double cost;
        fscanf(inputFile, "%d %d %le\n", &cityA, &cityB, &cost);
        tsp->roadCosts[cityA][cityB] = cost;
        tsp->roadCosts[cityB][cityA] = cost;
    }

    fclose(inputFile);
    tspInitializeMinCosts(tsp);
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
        exit(EXIT_FAILURE);
        ;
    }

    const char* inPath = argv[1];
    double maxTourCost = atoi(argv[2]);
    LOG("inPath = %s", inPath);
    LOG("maxTourCost = %f", maxTourCost);
    tsp_t* tsp = parseInput(inPath);
    DEBUG(tspPrint(tsp));

    double execTime = -omp_get_wtime();
    tspSolution_t* solution = tspSolve(tsp, maxTourCost);
    execTime += omp_get_wtime();

    fprintf(stderr, "%.1fs\n", execTime);
    printSolution(tsp, solution);

    tspSolutionDestroy(solution);
    tspDestroy(tsp);
    return 0;
}