#include "include.h"
#include "tsp/tspApi.h"
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

void processMasterProc(int argc, char* argv[], tspApi_t* api) {
    if (argc != 3) {
        printf("Usage: ./tsp <cities_file> <max_value>\n");
        exit(EXIT_FAILURE);
    }

    const char* inPath = argv[1];
    double maxTourCost = atoi(argv[2]);
    LOG("inPath = %s", inPath);
    LOG("maxTourCost = %f", maxTourCost);
    tsp_t* tsp = parseInput(inPath);
    DEBUG(tspPrint(tsp));

    MPI_Bcast(tsp, 1, api->problem_t, 0, MPI_COMM_WORLD);
    MPI_Bcast(&maxTourCost, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double execTime = -omp_get_wtime();
    tspSolution_t* solution = tspSolve(api, tsp, maxTourCost);
    execTime += omp_get_wtime();

    fprintf(stderr, "%.1fs\n", execTime);
    printSolution(tsp, solution);
    tspSolutionDestroy(solution);
    tspDestroy(tsp);
}

void processTaskProc(tspApi_t* api) {
    tsp_t* tsp = tspCreate();
    double maxTourCost = 0;

    MPI_Bcast(tsp, 1, api->problem_t, 0, MPI_COMM_WORLD);
    MPI_Bcast(&maxTourCost, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    tspSolution_t* solution = tspSolve(api, tsp, maxTourCost);
    tspSolutionDestroy(solution);
    tspDestroy(tsp);
}

int main(int argc, char* argv[]) {
    tspApi_t* api = tspApiInit();

    if (api->procType == PROCTYPE_MASTER)
        processMasterProc(argc, argv, api);
    else
        processTaskProc(api);

    tspApiTerminate(api);
    return 0;
}