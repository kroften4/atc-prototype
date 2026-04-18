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

#define EXIT_ALTITUDE    9
#define COLLISION_RADIUS 1

struct state
{
    struct plane *planes;
    struct endpoint *endpoints;
    struct vec bounds;
    uint8_t num_endpoints;
    uint8_t num_planes;
};

/**
 * Flight end status, sorted by priority (low to high).
 * If multiple statuses detected on a plane, the highest should win, e. g. a
 * plane exits at a wrong altitude but also crashed -> FLE_CRASH wins.
 * Use _FLE_LAST to iterate over the enum values.
 */
enum flight_status : uint8_t
{
    FLE_IN_PROCESS = 0,
    FLE_SUCCESS,

    FLE_OUT_OF_FUEL,

    FLE_WRONG_ALTITUDE,

    FLE_EXIT_NOT_LAND,
    FLE_LAND_NOT_EXIT,
    FLE_LAND_WRONG_DIR,
    FLE_WRONG_AIRPORT,
    FLE_WRONG_EXIT,

    FLE_ILLEGAL_EXIT,

    FLE_CRASH,
    FLE_COLLISION,

    _FLE_LAST ///< Used to iterate over enum vals
};

struct flight_end_data
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
 * Set `fle_data.type` if `status` is higher in priority.
 * @retval true if `status` was applied.
 */
bool fle_status_try_set(struct flight_end_data *fle_data,
						enum flight_status status);

/**
 * Checks flight status of plane over endpoint.
 *
 * @attention ONLY CALL AFTER GENERAL FLIGHT STATUS CHECKING (crashing, fuel, bounds)
 */
enum flight_status plane_at_endpoint_status(struct plane plane,
                                            struct endpoint endpoint);

/**
 * Check if a plane has reached a flight end.
 *
 * @attention FLE_COLLISION must be checked before calling this function (see
 * @ref arena_check_collision)
 *
 * @param[in] state game state
 * @param[in] plane plane to check for
 * @param[out] res flight end reason, left unchanged if did not detect flight end
 * @retval true if detected that `plane` reached a flight end and set `res`.
 */
bool plane_check_flight_end(struct state *state, struct plane plane,
                            struct flight_end_data *res);

/**
 * Check collision between 2 planes on the arena.
 */
bool plane_check_collision(struct plane p1, struct plane p2);

/**
 * Check if any plane collision on the arena has happened.
 * @warning This check is separated and should be done before any other
 * end-of-game checks. This is because we check all *pairs* of planes, not
 * one-by-one.
 *
 * @param [in] state game state
 * @param [out] res flight end reason, left unchanged if did not detect flight end
 * @retval true if detected a collision between 2 planes and set `res`.
 */
bool arena_check_collision(struct state *state, struct flight_end_data *res);

/**
 * Check end of game condition
 * @retval true if the game should be ended and set `res`
 */
bool arena_check_end_of_game(struct state *state, struct flight_end_data *res);

#endif // __STATE_H__
