#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define _THROW_NOT_YET_IMPLEMENTED()                               \
    do {                                                           \
        (void)fprintf(stderr, "%s not yet implemented", __func__); \
        exit(42);                                                  \
    } while (0)

#endif // __UTILS_H__
