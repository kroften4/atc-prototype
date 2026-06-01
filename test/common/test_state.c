#include "atc/dir.h"
#include "atc/plane.h"
#include "atc/state.h"
#include "test/test_state.h"

#include "atc/utils.h"
#include <stdint.h>
#include <unity.h>

// TODO: test arena_check_collision
// corners around (5,4)
TEST_CASE(PL(5, 4, 5), PL(3, 2, 5))
TEST_CASE(PL(5, 4, 5), PL(7, 2, 5))
TEST_CASE(PL(5, 4, 5), PL(3, 6, 5))
TEST_CASE(PL(5, 4, 5), PL(7, 6, 5))
// in range at (x,y), but not altitude
TEST_CASE(PL(5, 4, 6), PL(4, 5, 4))
TEST_CASE(PL(5, 4, 6), PL(4, 5, 8))
TEST_CASE(PL(5, 4, 2), PL(3, 4, 0))
TEST_CASE(PL(5, 4, 3), PL(5, 6, 5))
void test_plane_check_collision_DontCollide(struct plane p1, struct plane p2)
{
	TEST_ASSERT_FALSE(plane_check_collision(p1, p2));
}

// around (7,8)
// TODO: TEST_MATRIX might be better?
TEST_CASE(PL(7, 8, 7), PL(6, 7, 6))
TEST_CASE(PL(7, 8, 7), PL(7, 7, 7))
TEST_CASE(PL(7, 8, 7), PL(8, 7, 8))
TEST_CASE(PL(7, 8, 7), PL(6, 8, 8))
TEST_CASE(PL(7, 8, 7), PL(7, 8, 7))
TEST_CASE(PL(7, 8, 7), PL(8, 8, 6))
TEST_CASE(PL(7, 8, 7), PL(6, 9, 7))
TEST_CASE(PL(7, 8, 7), PL(7, 9, 8))
TEST_CASE(PL(7, 8, 7), PL(8, 9, 6))
void test_plane_check_collision_DoCollide(struct plane p1, struct plane p2)
{
	TEST_ASSERT_TRUE(plane_check_collision(p1, p2));
}

// TODO: should landing at airport with no fuel count as success
TEST_CASE(PL_DIR(39, 13, 3, DIR_45), EP(EP_AIRPORT, 50, 10, DIR_45),
		  FLE_IN_PROCESS, false, "Nothing is happening")
TEST_CASE(PL_DIR(39, 13, 1, DIR_45), EP(EP_AIRPORT, 39, 13, DIR_45),
		  FLE_IN_PROCESS, false, "Did not land yet")
TEST_CASE(PL_DIR(39, 13, 0, DIR_45), EP(EP_AIRPORT, 39, 13, DIR_45),
		  FLE_SUCCESS, false, "Landing at airport; dont end game")
TEST_CASE(PL_DIR(39, 13, 0, DIR_180), EP(EP_AIRPORT, 39, 13, DIR_45),
		  FLE_LAND_WRONG_DIR, true, "")
TEST_CASE(PL_DIR(39, 13, 9, DIR_45), EP(EP_EXIT, 39, 13, DIR_225), FLE_SUCCESS,
		  false, "Exiting legally at any angle; dont end game")
TEST_CASE(PL_DIR(39, 13, 7, DIR_45), EP(EP_EXIT, 39, 13, DIR_45),
		  FLE_WRONG_ALTITUDE, true, "")
// in bsd-games2 package leaving at 0 altitude at any exit says wrong
// altitude, leaving illegally says crash
TEST_CASE(PL_DIR(39, 13, 0, DIR_45), EP(EP_EXIT, 39, 13, DIR_45), FLE_CRASH,
		  true, "Crashed; not just wrong altitude")
TEST_CASE(PL_DIR(39, 12, 0, DIR_0), EP(EP_AIRPORT, 39, 13, DIR_0), FLE_CRASH,
		  true, "")
TEST_CASE(PL_FULL(20, 15, 0, .dir = DIR_90, .fuel = 0),
		  EP(EP_AIRPORT, 20, 15, DIR_90), FLE_SUCCESS, false,
		  "Ran out of fuel but landed")
TEST_CASE(PL_FULL(19, 15, 4, .dir = DIR_90, .fuel = 0),
		  EP(EP_AIRPORT, 20, 15, DIR_90), FLE_OUT_OF_FUEL, true, "")
void test_arena_check_end_of_game_FlightEnd_EndpointIsDestination(
	struct plane plane, struct endpoint endpoint,
	enum flight_status expected_status, bool should_end_game, char *message)
{
	plane.destination = &endpoint;
	struct plane planes[] = { plane };
	struct endpoint endpoints[] = { endpoint };
	struct state state = {
		.bounds = { 100, 100 },
		.planes = planes,
		.num_planes = ARRLEN(planes),
		.endpoints = endpoints,
		.num_endpoints = ARRLEN(endpoints),
	};
	struct flight_end_data res = {};
	// TEST_ASSERT_MESSAGE(
	// 	arena_check_end_of_game(&state, &res) == should_end_game, message);
	TEST_ASSERT(arena_check_end_of_game(&state, &res) == should_end_game);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected_status, res.type, message);
}
