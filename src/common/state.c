#include "atc/plane.h"
#include "atc/vec.h"
#include "atc/state.h"
#include <stdint.h>
#include <stdlib.h>

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
		for (size_t j = i + 1; i < state->num_planes; i++) {
			struct plane p1 = state->planes[i];
			struct plane p2 = state->planes[j];
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
	if (arena_check_collision(state, res))
		return true;
	for (size_t i = 0; i < state->num_planes; i++) {
		struct plane plane = state->planes[i];
		if (plane_check_flight_end(state, plane, res)) {
			return true;
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

	if (over_endpoint && endpoint.type != EP_AIRPORT && plane.altitude == 0) {
		res->type = FLE_CRASH;
		return true;
	}

	if (!vec_in_bounds(plane.pos, state->bounds.x, state->bounds.y)) {
		res->type = FLE_ILLEGAL_EXIT;
		return true;
	}

	bool landing_at_airport = over_endpoint && endpoint.type == EP_AIRPORT &&
							  plane.altitude == 0;
	bool over_exit = over_endpoint && endpoint.type == EP_EXIT;
    bool reached_endpoint = landing_at_airport || over_exit;

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

	if (over_exit && plane.altitude != EXIT_ALTITUDE) {
		res->type = FLE_WRONG_ALTITUDE;
        return true;
	}

	if (plane.fuel == 0) {
		res->type = FLE_OUT_OF_FUEL;
		return true;
	}

    // Now its FLE_SUCCESS or FLE_IN_PROCESS

	if (over_exit && plane.altitude == EXIT_ALTITUDE) {
		res->type = EXIT_SUCCESS;
        return false;
	}

	if (landing_at_airport && endpoint.dir == plane.dir) {
		res->type = EXIT_SUCCESS;
        return false;
	}

	res->type = FLE_IN_PROCESS;
	return false;
}
