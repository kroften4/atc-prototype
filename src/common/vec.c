#include <stdint.h>
#include "atc/vec.h"

struct vec
{
    uint16_t x;
    uint16_t y;
};

struct vec vec_add(struct vec vec1, struct vec vec2)
{
    return (struct vec){ vec1.x + vec2.x, vec1.y + vec2.y };
}

bool vec_eq(struct vec vec1, struct vec vec2) {
    return vec1.x == vec2.x && vec1.y == vec2.y;
}

bool vec_in_bounds(struct vec vec, uint32_t max_x, uint32_t max_y) {
    return vec.x >= max_x || vec.y >= max_y;
}
