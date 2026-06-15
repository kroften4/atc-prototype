#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "atc/vec.h"
#include "atc/dir.h"
#include <stdlib.h>
#include "atc/plane.h"

#define PROP_BUF 2
#define JET_BUF 1

#define MAX_TURN_DELTA 2

#define EXIT_ENTRY_ALTITUDE 7

void plane_init(struct plane *plane, struct endpoint *origin,
				struct endpoint *destination)
{
	plane->type = rand() % PLANE_TYPES_AMOUNT;
	plane->pos = origin->pos;
	plane->dir = origin->dir;
	switch (origin->type) {
	case EP_AIRPORT:
		plane->altitude = 0;
		plane->comm.type = COMM_HOLD;
		break;
	case EP_EXIT:
		plane->altitude = EXIT_ENTRY_ALTITUDE;
		plane->comm.type = COMM_NONE;
		break;
	}
	plane->target_altitude = plane->altitude;
	plane->fuel = PLANE_START_FUEL;
	plane->mark = MS_MARKED;
	plane->is_active = true;
	plane->left_origin = false;
	plane->destination = destination;
}

void plane_comm_circle(struct plane *plane, enum circle_dir circle_dir)
{
	plane->comm.type = COMM_CIRCLE;
	plane->comm.data.circle_dir = circle_dir;
}

void plane_comm_turn(struct plane *plane, dir_t dir)
{
	plane->comm.type = COMM_TURN;
	plane->comm.data.target_dir = dir;
}

void plane_comm_turn_left(struct plane *plane, dir_t angle)
{
	dir_t abs_dir = dir_sub(plane->dir, angle);
	plane_comm_turn(plane, abs_dir);
}

void plane_comm_turn_right(struct plane *plane, dir_t angle)
{
	dir_t abs_dir = dir_add(plane->dir, angle);
	plane_comm_turn(plane, abs_dir);
}

void plane_move(struct plane *plane)
{
	// TODO: climb/descend to target_altitude
	if (plane->comm.at_beacon != NULL) {
		plane->pos = vec_add(plane->pos, dir_to_vec(plane->dir));
		plane->left_origin = true;
		return;
	}
	switch (plane->comm.type) {
	case COMM_NONE:
		break;
	case COMM_HOLD:
		return;
	case COMM_CIRCLE:
		if (plane->comm.data.circle_dir == CDIR_CW) {
			plane->dir = dir_add(plane->dir, MAX_TURN_DELTA);
		} else {
			plane->dir = dir_sub(plane->dir, MAX_TURN_DELTA);
		}
		break;
	case COMM_TURN:
		dir_t delta_cw = dir_sub(plane->comm.data.target_dir, plane->dir);
		if (delta_cw == 0) {
			plane->comm.type = COMM_NONE;
			break;
		}
		dir_t delta_ccw = dir_sub(plane->dir, plane->comm.data.target_dir);

		if (delta_cw <= delta_ccw) {
			plane->dir = dir_add(plane->dir, delta_cw > MAX_TURN_DELTA ?
												 MAX_TURN_DELTA :
												 delta_cw);
		} else {
			plane->dir = dir_sub(plane->dir, delta_ccw > MAX_TURN_DELTA ?
												 MAX_TURN_DELTA :
												 delta_ccw);
		}
		break;
	}
	plane->pos = vec_add(plane->pos, dir_to_vec(plane->dir));
	plane->left_origin = true;
}
