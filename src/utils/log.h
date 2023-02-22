#ifndef __UTILS_LOG_H__
#define __UTILS_LOG_H__

#ifdef __DEBUG__
#define LOG(X, ...) printf("[Debug]: " #X "\n", __VA_ARGS__)
#define DEBUG(X)                 \
    printf("[DEBUG]: " #X "\n"); \
    X
#else
#define LOG(X, ...)
#define DEBUG(X)
#endif

#endif