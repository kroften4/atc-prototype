#ifndef __DIR_H__
#define __DIR_H__

#include "atc/vec.h"
#include <stdint.h>

#define DIR_0   0
#define DIR_45  1
#define DIR_90  2
#define DIR_135 3
#define DIR_180 4
#define DIR_225 5
#define DIR_270 6
#define DIR_315 7

#define NUM_DIRS 8

typedef uint8_t dir_t;

struct vec dir_to_vec(dir_t dir);

dir_t dir_add(dir_t dir1, dir_t dir2);

dir_t dir_sub(dir_t dir1, dir_t dir2);

#endif // __DIR_H__
