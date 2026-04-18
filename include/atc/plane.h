#ifndef __PLANE_H__
#define __PLANE_H__

#include "dir.h"
#include "vec.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum mark_status : uint8_t
{
    MS_UNMARKED,
    MS_MARKED,
    MS_IGNORED
};

enum plane_type : uint8_t
{
    PLANE_JET,
    PLANE_PROP,
};

enum comm_type : uint8_t
{
    COMM_NONE,
    COMM_HOLD,
    COMM_CIRCLE,
    COMM_TURN,
};

enum circle_dir : uint8_t
{
    CDIR_CW,
    CDIR_CCW,
};

union comm_data
{
    dir_t target_dir;
    enum circle_dir circle_dir;
};

struct comm
{
    enum comm_type type;
    union comm_data data;
};

enum endpoint_type : uint8_t
{
    EP_AIRPORT,
    EP_EXIT
};

struct endpoint
{
    struct vec pos;
    enum endpoint_type type;
    dir_t dir;
};

struct plane
{
    struct endpoint *destination;
    struct vec pos;
    struct comm comm;
    enum mark_status mark;
    enum plane_type type;
    dir_t dir;
    uint8_t pos_buffer;
    uint8_t altitude;
    uint8_t target_altitide;
    uint8_t fuel;
    bool is_active;
};

void plane_spawn(struct plane *plane, struct endpoint endpoint);

void plane_comm_circle(struct plane *plane, enum circle_dir circle_dir);

void plane_comm_turn(struct plane *plane, dir_t dir);

void plane_comm_turn_left(struct plane *plane, dir_t angle);

void plane_comm_turn_right(struct plane *plane, dir_t angle);

void plane_move(struct plane *plane);

void plane_update(struct plane *plane);

#endif // __PLANE_H__
