#ifndef __UTILS__LOG_H__
#define __UTILS__LOG_H__

#ifdef __DEBUG__
#define MARK(X) printf("[DEBUG]: " #X "\n")
#define LOG(X, ...) printf("[Debug]: " #X "\n", __VA_ARGS__)
#define DEBUG(X)                 \
    printf("[DEBUG]: " #X "\n"); \
    X
#else
#define MARK(X)
#define LOG(X, ...)
#define DEBUG(X)
#endif

#endif // __UTILS__LOG_H__