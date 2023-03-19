#ifndef __UTILS_FILE_H__
#define __UTILS_FILE_H__

#include "include.h"

FILE* openFile(const char* path, const char* mode);

void closeFile(FILE* file);

#endif // __UTILS_FILE_H__