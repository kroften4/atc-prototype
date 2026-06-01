#include <stdint.h>
#include "atc/vec.h"

struct vec vec_add(struct vec vec1, struct vec vec2)
{
	return (struct vec){ vec1.x + vec2.x, vec1.y + vec2.y };
}

bool vec_eq(struct vec vec1, struct vec vec2)
{
	return vec1.x == vec2.x && vec1.y == vec2.y;
}
