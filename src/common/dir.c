#include <assert.h>
#include <stdint.h>
#include "atc/vec.h"
#include "atc/dir.h"

// TODO: make this enum
typedef uint8_t dir_t;

static struct vec dir_to_vec_lookup[] = { { 0, -1 }, { 1, -1 }, { 1, 0 },
										  { 1, 1 },	 { 0, 1 },	{ -1, 1 },
										  { -1, 0 }, { -1, -1 } };

struct vec dir_to_vec(dir_t dir)
{
	return dir_to_vec_lookup[dir];
}

dir_t char_to_dir(char ch)
{
	switch (ch) {
	case 'w':
		return DIR_0;
	case 'e':
		return DIR_45;
	case 'd':
		return DIR_90;
	case 'c':
		return DIR_135;
	case 'x':
		return DIR_180;
	case 'z':
		return DIR_225;
	case 'a':
		return DIR_270;
	case 'q':
		return DIR_315;
	default:
		assert("Invalid direction character");
		return -1;
	}
}

int dir_to_int_angle(dir_t dir)
{
	switch (dir) {
	case DIR_0:
		return 0;
	case DIR_45:
		return 45;
	case DIR_90:
		return 90;
	case DIR_135:
		return 135;
	case DIR_180:
		return 180;
	case DIR_225:
		return 225;
	case DIR_270:
		return 270;
	case DIR_315:
		return 315;
	default:
		assert("Invalid direction");
		return -1;
	}
}

dir_t dir_add(dir_t dir1, dir_t dir2)
{
	return (dir1 + dir2) % NUM_DIRS;
}

dir_t dir_sub(dir_t dir1, dir_t dir2)
{
	return (dir1 - dir2 + NUM_DIRS) % NUM_DIRS;
}
