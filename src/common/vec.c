#include <stdint.h>
#include "vec.h"

struct vec
{
    uint16_t x;
    uint16_t y;
};

struct vec vec_add(struct vec vec1, struct vec vec2)
{
    return (struct vec){ vec1.x + vec2.x, vec1.y + vec2.y };
}
