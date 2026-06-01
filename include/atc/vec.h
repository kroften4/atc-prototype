#ifndef __VEC_H__
#define __VEC_H__

#include <stdint.h>

struct vec
{
    int x;
    int y;
};

struct vec vec_add(struct vec vec1, struct vec vec2);

bool vec_eq(struct vec vec1, struct vec vec2);

#endif // __VEC_H__
