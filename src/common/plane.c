#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "atc/vec.h"
#include "atc/dir.h"
#include "atc/plane.h"

#define PROP_BUF 1
#define JET_BUF 0

#define EXIT_ENTRY_ALTITUDE 7

void plane_spawn(struct plane *plane, struct endpoint endpoint)
{
	plane->pos = endpoint.pos;
	plane->dir = endpoint.dir;
	switch (endpoint.type) {
	case EP_AIRPORT:
		plane->altitude = 0;
		break;
	case EP_EXIT:
		plane->altitude = EXIT_ENTRY_ALTITUDE;
		break;
	}
}

void plane_comm_circle(struct plane *plane, enum circle_dir circle_dir)
{
	plane->comm.type = COMM_CIRCLE;
	plane->comm.data.circle_dir = circle_dir;
}

void plane_comm_turn(struct plane *plane, dir_t dir)
{
	plane->comm.type = COMM_CIRCLE;
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
	plane->pos = vec_add(plane->pos, dir_to_vec(plane->dir));
}

void plane_update(struct plane *plane)
{
	plane->pos_buffer++;
	uint8_t maxbuf = 0;
	switch (plane->type) {
	case PLANE_JET:
		maxbuf = JET_BUF;
		break;
	case PLANE_PROP:
		maxbuf = PROP_BUF;
		break;
	}
	assert(plane->pos_buffer <= maxbuf);
	if (plane->pos_buffer == maxbuf) {
		plane_move(plane);
	}
}
