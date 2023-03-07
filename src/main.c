#include "include.h"
#include "tsp/tsp.h"
#include "utils/file.h"

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
    return tsp;
}

void printSolution(const tspSolution_t *solution) {
    if (solution->hasSolution) {
        printf("%.1f\n", solution->cost);
        for(size_t i=0; i<solution->bestTour->size; i++){
            if (i == solution->bestTour->size-1) printf("%d", solution->bestTour->tour[i]);
            else printf("%d ", solution->bestTour->tour[i]);
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

    const char* inPath = argv[1];
    int maxValue = atoi(argv[2]);
    LOG("inPath = %s", inPath);
    LOG("maxValue = %d", maxValue);

    tsp_t tsp = parseInput(inPath);
    DEBUG(tspPrint(&tsp));

    double execTime = -omp_get_wtime();
    tspSolve(&tsp, maxValue);
    execTime += omp_get_wtime();

    fprintf(stderr, "%.1fs\n", execTime);
    printSolution(&tsp.solution);
    tspDestroy(&tsp);
    return 0;
}