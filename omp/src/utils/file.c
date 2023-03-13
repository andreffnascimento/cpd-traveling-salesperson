#include "file.h"

FILE* openFile(const char* path, const char* mode) {
    FILE* file = fopen(path, mode);
    if (file == NULL) {
        printf("Unable to open the file: %s\n", path);
        exit(1);
    }

    return file;
}

void closeFile(FILE* file) {
    fclose(file);
}
