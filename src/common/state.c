#include "atc/plane.h"
#include "atc/vec.h"
#include "atc/state.h"
#include <stdint.h>

enum flight_status plane_at_endpoint_status(struct plane plane,
											struct endpoint endpoint)
{
	assert(vec_eq(plane.pos, endpoint.pos) && "plane not over endpoint");
	bool crashed = endpoint.type == EP_EXIT && plane.altitude == 3;
	assert(!crashed && "crashing not prechecked");
	assert(plane.fuel > 0 && "out of fuel not prechecked");

	if (endpoint.type == EP_AIRPORT && plane.altitude != 0) {
		return FLE_IN_PROCESS;
	}

	if (plane.destination->type != endpoint.type) {
		return plane.destination == EP_AIRPORT ? FLE_EXIT_NOT_LAND :
												 FLE_LAND_NOT_EXIT;
	}
	if (!vec_eq(plane.destination->pos, endpoint.pos)) {
		return plane.destination == EP_AIRPORT ? FLE_WRONG_AIRPORT :
												 FLE_WRONG_EXIT;
	}

	if (endpoint.type == EP_EXIT && plane.altitude != EXIT_ALTITUDE) {
		return FLE_WRONG_ALTITUDE;
	}

	bool landing_success = endpoint.type == EP_AIRPORT && plane.altitude == 0;
	bool exit_success = endpoint.type == EP_EXIT &&
						plane.altitude == EXIT_ALTITUDE;
	assert((landing_success || exit_success) &&
		   "did not detect flight end cause of plane at endpoint");
	return FLE_IN_PROCESS;
}

struct flight_end_status plane_check_flight_end(struct state *state,
												struct plane plane)
{
	struct flight_end_status res = { .plane = plane };
	for (size_t i = 0; i < MAX_ENDPOINTS; i++) {
		struct endpoint ep = state->endpoints[i];
		if (vec_eq(plane.pos, ep.pos)) {
			res.type = plane_at_endpoint_status(plane, ep);
			if (res.type == FLE_IN_PROCESS)
				continue;
			break;
		}
	}
	return res;
}
