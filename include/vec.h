#include <stdint.h>

struct vec
{
    uint16_t x;
    uint16_t y;
};

struct vec vec_add(struct vec vec1, struct vec vec2);
