/** @file
 * @brief Stateful game logic.
 */

#ifndef __STATE_H__
#define __STATE_H__

#include "atc/plane.h"
#include "atc/vec.h"
#include <stdint.h>

#define MAX_PLAINS    44
#define MAX_ENDPOINTS 64

#define EXIT_ALTITUDE 9

struct state
{
    struct plane planes[MAX_PLAINS];
    struct endpoint endpoints[MAX_ENDPOINTS];
    struct vec bounds;
};

enum flight_status : uint8_t
{
    FLE_IN_PROCESS,
    FLE_SUCCESS,
    FLE_ILLEGAL_EXIT,
    FLE_WRONG_ALTITUDE,
    FLE_WRONG_EXIT,
    FLE_WRONG_AIRPORT,
    FLE_LAND_NOT_EXIT,
    FLE_EXIT_NOT_LAND,
    FLE_COLLISION,
    FLE_CRASH,
    FLE_OUT_OF_FUEL,
};

struct flight_end_status
{
    struct plane plane;
    enum flight_status type;
    union
    {
        struct plane coll_plane;
        struct endpoint wrong_dest;
        uint8_t wrong_altitude;
    } data;
};

/**
 * Checks flight status of plane over endpoint.
 *
 * @attention ONLY CALL AFTER GENERAL FLIGHT STATUS CHECKING (crashing, fuel, bounds)
 */
enum flight_status plane_at_endpoint_status(struct plane plane,
                                            struct endpoint endpoint);

/**
 * Check if a plane has reached a flight end.
 */
struct flight_end_status plane_check_flight_end(struct state *state,
                                                struct plane plane);

#endif // __STATE_H__
