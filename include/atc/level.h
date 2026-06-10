#ifndef __ATC_LEVEL_H__
#define __ATC_LEVEL_H__

#include "atc/vec.h"
#include <stddef.h>

struct path
{
    struct vec start;
    struct vec end;
};

struct level
{
    struct endpoint *airports;
    size_t num_airports;
    struct endpoint *exits;
    size_t num_exits;
    struct beacon *beacons;
    size_t num_beacons;
    struct path *paths;
    size_t num_paths;
    struct vec bounds;
    size_t tick_speed;
    size_t max_plane_interval;
    size_t update_interval;
};

#endif // __ATC_LEVEL_H__
