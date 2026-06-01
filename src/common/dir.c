#include <stdint.h>
#include "atc/vec.h"
#include "atc/dir.h"

typedef uint8_t dir_t;

static struct vec dir_to_vec_lookup[] = { { 0, -1 }, { 1, -1 }, { 1, 0 },
										  { 1, 1 },	 { 0, 1 },	{ -1, 1 },
										  { -1, 0 }, { -1, -1 } };

struct vec dir_to_vec(dir_t dir)
{
	return dir_to_vec_lookup[dir];
}

dir_t dir_add(dir_t dir1, dir_t dir2)
{
	return (dir1 + dir2) % NUM_DIRS;
}

dir_t dir_sub(dir_t dir1, dir_t dir2)
{
	return (dir1 - dir2 + NUM_DIRS) % NUM_DIRS;
}
