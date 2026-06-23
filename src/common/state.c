#include "atc/level.h"
#include "atc/plane.h"
#include "atc/vec.h"
#include "atc/state.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SPAWN_SAFE_DISTANCE 4

bool arena_spawn_plane(struct state *state)
{
	if (rand() % state->spawn_coeff != 0) {
		return false;
	}

	// get a new plane
	struct plane *plane = NULL;
	size_t plane_idx = 0;
	for (size_t i = 0; i < state->num_planes; i++) {
		if (!state->planes[i].is_active) {
			plane = &state->planes[i];
			plane_idx = i;
			break;
		}
	}
	if (plane == NULL) {
		return false;
	}

	size_t origin_idx = rand() % state->num_endpoints;
	struct endpoint *origin = &state->endpoints[origin_idx];
	bool spawning_plane = false;
	for (size_t i = 0; i < state->num_endpoints; i++) {
		if (origin->type == EP_AIRPORT) {
			spawning_plane = true;
			break;
		}
		plane->pos.x = origin->pos.x;
		plane->pos.y = origin->pos.y;
		plane->altitude = 7;
		bool too_close = false;
		for (size_t i = 0; i < state->num_planes; i++) {
			struct plane *p2 = &state->planes[i];
			if (!p2->is_active)
				continue;
			if (plane_too_close(plane, p2, SPAWN_SAFE_DISTANCE)) {
				too_close = true;
				break;
			}
		}
		if (too_close) {
			origin_idx = (origin_idx + 1) % state->num_endpoints;
			origin = &state->endpoints[origin_idx];
			continue;
		}
		spawning_plane = true;
		break;
	}
	if (!spawning_plane)
		return false;

	size_t dest_idx = rand() % state->num_endpoints;
	if (origin_idx == dest_idx) {
		dest_idx = (dest_idx + 1) % state->num_endpoints;
	}
	struct endpoint *destination = &state->endpoints[dest_idx];
	assert(origin != destination && "there might only be one endpoint");

	plane_init(plane, plane_idx, origin, destination,
			   (size_t)state->bounds.x + state->bounds.y);
	origin = &state->endpoints[origin_idx];
	return true;
}

void state_init(struct state *state, struct level *level)
{
	state->spawn_coeff = level->spawn_coeff;
	state->update_interval = level->update_interval;

	state->num_paths = level->num_paths;
	state->paths = calloc(state->num_paths, sizeof(struct path));
	for (size_t i = 0; i < state->num_paths; i++) {
		state->paths[i].start.x = level->paths[i].start.x;
		state->paths[i].start.y = level->paths[i].start.y;
		state->paths[i].end.x = level->paths[i].end.x;
		state->paths[i].end.y = level->paths[i].end.y;
	}

	state->num_endpoints = level->num_airports + level->num_exits;
	state->endpoints = calloc(state->num_endpoints, sizeof(struct endpoint));
	size_t ep_idx = 0;
	for (size_t i = 0; i < level->num_airports; i++) {
		struct endpoint *ep = &state->endpoints[ep_idx];
		*ep = level->airports[i];
		ep->num = i;
		ep->type = EP_AIRPORT;
		ep_idx++;
	}
	for (size_t i = 0; i < level->num_exits; i++) {
		struct endpoint *ep = &state->endpoints[ep_idx];
		*ep = level->exits[i];
		ep->num = i;
		ep->type = EP_EXIT;
		ep_idx++;
	}
	assert(ep_idx < MAX_ENDPOINTS);

	state->num_beacons = level->num_beacons;
	assert(state->num_beacons < MAX_BEACONS);
	state->beacons = calloc(state->num_beacons, sizeof(struct beacon));
	for (size_t i = 0; i < state->num_beacons; i++) {
		struct beacon *bcn = &state->beacons[i];
		*bcn = level->beacons[i];
		bcn->num = i;
	}

	state->planes = calloc(MAX_PLANES, sizeof(struct plane));
	state->num_planes = MAX_PLANES;
	for (size_t i = 0; i < MAX_PLANES; i++) {
		struct plane *plane = &state->planes[i];
		plane->is_active = false;
	}

	state->bounds.x = level->bounds.x;
	state->bounds.y = level->bounds.y;
}

void state_deinit(struct state *state)
{
	free(state->planes);
	free(state->endpoints);
	free(state->beacons);
}

void plane_check_if_at_beacon(struct state *state, struct plane *plane)
{
	if (plane->comm.at_beacon == NULL) {
		return;
	}
	for (size_t i = 0; i < state->num_beacons; i++) {
		struct beacon *bcn = &state->beacons[i];
		if (bcn != plane->comm.at_beacon) {
			continue;
		}
		if (vec_eq(plane->pos, bcn->pos)) {
			plane->comm.at_beacon = NULL;
			if (plane->mark == MS_UNMARKED) {
				plane->mark = MS_MARKED;
			}
			return;
		}
	}
}

void arena_check_if_at_beacon(struct state *state)
{
	for (size_t i = 0; i < state->num_planes; i++) {
		struct plane *plane = &state->planes[i];
		plane_check_if_at_beacon(state, plane);
	}
}

bool plane_too_close(struct plane *p1, struct plane *p2, int distance)
{
	bool z_overlap = abs(p1->altitude - p2->altitude) <= distance;
	bool x_overlap = abs(p1->pos.x - p2->pos.x) <= distance;
	bool y_overlap = abs(p1->pos.y - p2->pos.y) <= distance;
	return z_overlap && x_overlap && y_overlap;
}

bool arena_check_collision(struct state *state, struct flight_end_data *res)
{
	for (size_t i = 0; i < state->num_planes - 1; i++) {
		struct plane *p1 = &state->planes[i];
		if (!p1->is_active || !p1->left_origin)
			continue;
		for (size_t j = i + 1; j < state->num_planes; j++) {
			struct plane *p2 = &state->planes[j];
			if (!p2->is_active || !p2->left_origin)
				continue;
			if (plane_too_close(p1, p2, COLLISION_RADIUS)) {
				// FIXME: no plane number is specified this way.
				res->type = FLE_COLLISION;
				res->plane_idx = i;
				res->data.coll_plane_idx = j;
				return true;
			}
		}
	}
	return false;
}

bool arena_check_end_of_game(struct state *state, struct flight_end_data *res)
{
	if (arena_check_collision(state, res)) {
		return true;
	}
	for (size_t i = 0; i < state->num_planes; i++) {
		struct plane *plane = &state->planes[i];
		if (!plane->is_active) {
			continue;
		}
		if (plane_check_flight_end(state, i, res)) {
			return true;
		}
		if (res->type == FLE_SUCCESS) {
			plane->is_active = false;
			state->planes_safe++;
		}
	}
	return false;
}

bool fle_status_try_set(struct flight_end_data *fle_data,
						enum flight_status status)
{
	if (fle_data->type > status)
		return false;

	fle_data->type = status;
	return true;
}

static bool plane_in_bounds(struct plane *plane, struct state *state)
{
	struct vec pos = plane->pos;
	return pos.x < state->bounds.x - 1 && pos.y < state->bounds.y - 1 &&
		   pos.x > 0 && pos.y > 0;
}

bool plane_check_flight_end(struct state *state, size_t plane_idx,
							struct flight_end_data *res)
{
	// FLE_COLLISION must be prechecked

	assert(plane_idx < state->num_planes);
	struct plane plane = state->planes[plane_idx];
	res->plane_idx = plane_idx;

	bool over_endpoint = false;
	struct endpoint endpoint = {};
	for (size_t i = 0; i < state->num_endpoints; i++) {
		endpoint = state->endpoints[i];
		if (vec_eq(plane.pos, endpoint.pos)) {
			over_endpoint = true;
			break;
		}
	}

	bool at_airport = over_endpoint && endpoint.type == EP_AIRPORT &&
					  plane.altitude == 0;
	bool landing_at_airport = at_airport && plane.left_origin;
	bool exiting = over_endpoint && endpoint.type == EP_EXIT &&
				   plane.left_origin;
	bool reached_endpoint = landing_at_airport || exiting;

	if (!at_airport && plane.altitude == 0) {
		res->type = FLE_CRASH;
		return true;
	}

	if (!over_endpoint && !plane_in_bounds(&plane, state)) {
		res->type = FLE_ILLEGAL_EXIT;
		return true;
	}

	if (reached_endpoint && !vec_eq(plane.destination->pos, endpoint.pos)) {
		res->type = plane.destination == EP_AIRPORT ? FLE_WRONG_AIRPORT :
													  FLE_WRONG_EXIT;
		return true;
	}

	if (reached_endpoint && plane.destination->type != endpoint.type) {
		res->type = plane.destination == EP_AIRPORT ? FLE_EXIT_NOT_LAND :
													  FLE_LAND_NOT_EXIT;
		return true;
	}

	if (landing_at_airport && endpoint.dir != plane.dir) {
		res->type = FLE_LAND_WRONG_DIR;
		return true;
	}

	if (exiting && plane.altitude != EXIT_ALTITUDE) {
		res->type = FLE_WRONG_ALTITUDE;
		return true;
	}

	if (!landing_at_airport && plane.fuel == 0) {
		res->type = FLE_OUT_OF_FUEL;
		return true;
	}

	// Now its FLE_SUCCESS or FLE_IN_PROCESS

	if (exiting && plane.altitude == EXIT_ALTITUDE) {
		res->type = FLE_SUCCESS;
		return false;
	}

	if (landing_at_airport && endpoint.dir == plane.dir) {
		res->type = FLE_SUCCESS;
		return false;
	}

	res->type = FLE_IN_PROCESS;
	return false;
}

void plane_update(struct state *state, struct plane *plane)
{
	switch (plane->type) {
	case PLANE_JET:
		plane_advance(plane);
		break;
	case PLANE_PROP:
		if (state->time % 2 == 0) {
			plane_advance(plane);
		}
		break;
	}
}

void arena_update_planes(struct state *state)
{
	for (size_t i = 0; i < state->num_planes; i++) {
		struct plane *plane = &state->planes[i];
		if (!plane->is_active)
			continue;
		plane_update(state, plane);
	}
}

bool arena_tick(struct state *state, struct flight_end_data *fle_data)
{
	state->time++;
	arena_spawn_plane(state);

	arena_update_planes(state);
	arena_check_if_at_beacon(state);
	return arena_check_end_of_game(state, fle_data);
}

struct plane *get_plane(struct state *state, size_t plane_num)
{
	if (plane_num >= state->num_planes) {
		return NULL;
	}
	struct plane *plane = &state->planes[plane_num];
	if (!plane->is_active) {
		return NULL;
	}
	return plane;
}
