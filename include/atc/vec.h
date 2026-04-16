#ifndef __VEC_H__
#define __VEC_H__

#include <stdint.h>

struct vec
{
    uint16_t x;
    uint16_t y;
};

struct vec vec_add(struct vec vec1, struct vec vec2);

bool vec_eq(struct vec vec1, struct vec vec2);

bool vec_in_bounds(struct vec vec, uint32_t max_x, uint32_t max_y);

#endif // __VEC_H__
