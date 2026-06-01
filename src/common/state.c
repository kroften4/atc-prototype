#include "atc/level.h"
#include "atc/plane.h"
#include "atc/vec.h"
#include "atc/state.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

bool arena_spawn_plane(struct state *state)
{
	// TODO: origin != destination
	// TODO: origin cooldown
	// TODO: endpoint popularity
	struct endpoint *origin = &state->endpoints[rand() % state->num_endpoints];
	struct endpoint *destination = NULL;
	do {
		destination = &state->endpoints[rand() % state->num_endpoints];
	} while (origin == destination);

	for (size_t i = 0; i < state->num_planes; i++) {
		struct plane *plane = &state->planes[i];
		if (!plane->is_active) {
			plane_init(plane, origin, destination);
			return true;
		}
	}
	return false;
}

void state_init(struct state *state, struct level *level)
{
	state->num_endpoints = level->num_airports + level->num_exits;
	state->endpoints = calloc(state->num_endpoints, sizeof(struct endpoint));
	size_t ep_idx = 0;
	for (size_t i = 0; i < level->num_airports; i++) {
		struct endpoint *ep = &state->endpoints[ep_idx];
		*ep = level->airports[i];
		ep->num = '0' + i;
		ep->type = EP_AIRPORT;
		ep_idx++;
	}
	for (size_t i = 0; i < level->num_exits; i++) {
		struct endpoint *ep = &state->endpoints[ep_idx];
		*ep = level->exits[i];
		ep->num = '0' + i;
		ep->type = EP_EXIT;
		ep_idx++;
	}

	state->num_beacons = level->num_beacons;
	state->beacons = calloc(state->num_beacons, sizeof(struct beacon));
	for (size_t i = 0; i < state->num_beacons; i++) {
		struct beacon *bcn = &state->beacons[i];
		*bcn = level->beacons[i];
		bcn->num = '0' + i;
	}

	state->planes = calloc(MAX_PLANES, sizeof(struct plane));
	state->num_planes = MAX_PLANES;
	for (size_t i = 0; i < 22; i++) {
		struct plane *plane = &state->planes[i];
		plane->letter = 'a' + i;
		plane->type = PLANE_JET;
		plane->pos_buffer = 0;
		plane->is_active = false;
		plane->fuel = PLANE_START_FUEL;
	}
	for (size_t i = 22; i < MAX_PLANES; i++) {
		struct plane *plane = &state->planes[i];
		plane->letter = 'A' + i;
		plane->type = PLANE_PROP;
		plane->pos_buffer = 0;
		plane->is_active = false;
		plane->fuel = PLANE_START_FUEL;
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
	for (size_t i = 0; i < state->num_endpoints; i++) {
		struct beacon *bcn = &state->beacons[i];
		if (bcn != plane->comm.at_beacon) {
			continue;
		}
		if (vec_eq(plane->pos, bcn->pos)) {
			plane->comm.at_beacon = NULL;
			return;
		}
	}
}

void arena_check_if_at_beacon(struct state *state)
{
	for (size_t i = 0; i < state->num_planes - 1; i++) {
		struct plane *plane = &state->planes[i];
		plane_check_if_at_beacon(state, plane);
	}
}

bool plane_check_collision(struct plane p1, struct plane p2)
{
	bool z_overlap = abs(p1.altitude - p2.altitude) <= COLLISION_RADIUS;
	bool x_overlap = abs(p1.pos.x - p2.pos.x) <= COLLISION_RADIUS;
	bool y_overlap = abs(p1.pos.y - p2.pos.y) <= COLLISION_RADIUS;
	return z_overlap && x_overlap && y_overlap;
}

bool arena_check_collision(struct state *state, struct flight_end_data *res)
{
	for (size_t i = 0; i < state->num_planes - 1; i++) {
		struct plane p1 = state->planes[i];
		if (!p1.is_active)
			continue;
		for (size_t j = i + 1; i < state->num_planes; i++) {
			struct plane p2 = state->planes[j];
			if (!p2.is_active)
				continue;
			if (plane_check_collision(p1, p2)) {
				res->type = FLE_COLLISION;
				res->plane = p1;
				res->data.coll_plane = p2;
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
		if (plane_check_flight_end(state, *plane, res)) {
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
	return pos.x < state->bounds.x && pos.y < state->bounds.y && pos.x >= 0 &&
		   pos.y >= 0;
}

bool plane_check_flight_end(struct state *state, struct plane plane,
							struct flight_end_data *res)
{
	// FLE_COLLISION must be prechecked

	res->plane = plane;

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

	if (!exiting && !plane_in_bounds(&plane, state)) {
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

	if (at_airport && endpoint.dir != plane.dir) {
		res->type = FLE_LAND_WRONG_DIR;
		return true;
	}

	if (exiting && plane.altitude != EXIT_ALTITUDE) {
		res->type = FLE_WRONG_ALTITUDE;
		return true;
	}

	if (!at_airport && plane.fuel == 0) {
		res->type = FLE_OUT_OF_FUEL;
		return true;
	}

	// Now its FLE_SUCCESS or FLE_IN_PROCESS

	if (exiting && plane.altitude == EXIT_ALTITUDE) {
		res->type = FLE_SUCCESS;
		return false;
	}

	if (at_airport && endpoint.dir == plane.dir) {
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
		plane_move(plane);
		break;
	case PLANE_PROP:
		if (state->time % 2 == 0) {
			plane_move(plane);
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
	size_t rnd = rand() % 100;
	size_t planes_spawned;
	if (rnd < 50) {
		planes_spawned = 0;
	} else if (rnd < 80) {
		planes_spawned = 1;
	} else {
		planes_spawned = 2;
	}
	for (size_t i = 0; i < planes_spawned; i++) {
		arena_spawn_plane(state);
	}

	arena_update_planes(state);
	arena_check_if_at_beacon(state);
	return arena_check_end_of_game(state, fle_data);
}
