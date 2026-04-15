#include <assert.h>
#include <stdint.h>
#include "vec.h"

#define PROP_BUF 1
#define JET_BUF 0

enum mark_status : uint8_t { MS_UNMARKED, MS_MARKED, MS_IGNORED };

enum plane_type : uint8_t {
	PLANE_JET,
	PLANE_PROP,
};

enum comm_type : uint8_t {
	COMM_NONE,
	COMM_CIRCLE,
	COMM_TURN,
};

union comm_data {
	struct vec target_dir;
	bool cw_circle;
};

struct comm {
	enum comm_type type;
	union comm_data data;
};

struct plane {
	struct vec pos;
	struct vec dir;
	struct comm comm;
	enum mark_status mark;
	enum plane_type type;
	uint8_t pos_buffer;
	uint8_t altitude;
	uint8_t target_altitide;
};

#define DIR_N (struct vec){ 0, 1 }
#define DIR_NE (struct vec){ 1, 1 }
#define DIR_E (struct vec){ 1, 0 }
#define DIR_SE (struct vec){ 1, -1 }
#define DIR_S (struct vec){ 0, -1 }
#define DIR_SW (struct vec){ -1, -1 }
#define DIR_W (struct vec){ -1, 0 }
#define DIR_NW (struct vec){ -1, 1 }

void plane_comm_circle_cw(struct plane *plane)
{
	plane->comm.type = COMM_CIRCLE;
	plane->comm.data.cw_circle = true;
}

void plane_comm_circle_ccw(struct plane *plane)
{
	plane->comm.type = COMM_CIRCLE;
	plane->comm.data.cw_circle = false;
}

void plane_comm_turn(struct plane *plane, struct vec dir)
{
	plane->comm.type = COMM_CIRCLE;
	plane->comm.data.target_dir = dir;
}

void plane_comm_turn_left(struct plane *plane)
{
    // TODO: maybe use circular dirs idk
    // struct vec dir = vec_add(plane->dir, a);
    // plane_comm_turn(plane, );
}

void plane_comm_turn_left90()
{
}

void plane_comm_turn_right()
{
}

void plane_comm_turn_right90()
{
}

void plane_move(struct plane *plane)
{
	plane->pos = vec_add(plane->pos, plane->dir);
}

void plane_update(struct plane *plane)
{
	plane->pos_buffer++;
	uint8_t maxbuf = 0;
	switch (plane->type) {
	case PLANE_JET:
		maxbuf = JET_BUF;
	case PLANE_PROP:
		maxbuf = PROP_BUF;
	}
	assert(plane->pos_buffer <= maxbuf);
	if (plane->pos_buffer == maxbuf) {
		plane_move(plane);
	}
}
