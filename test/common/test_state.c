#include "atc/plane.h"
#include "atc/state.h"
#include "test/test_state.h"

#include <unity.h>

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
