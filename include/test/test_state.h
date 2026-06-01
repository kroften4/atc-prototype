#ifndef __TEST_STATE_H__
#define __TEST_STATE_H__

// struct builders for convenience

#define PL(pos_x, pos_y, alt)                 \
    (struct plane) { .pos = { pos_x, pos_y }, \
                     .altitude = alt,         \
                     .fuel = 10,              \
                     .is_active = true,       \
                     .left_origin = true }

#define PL_DIR(pos_x, pos_y, alt, dir_)       \
    (struct plane) { .pos = { pos_x, pos_y }, \
                     .altitude = alt,         \
                     .dir = dir_,             \
                     .fuel = 10,              \
                     .is_active = true,       \
                     .left_origin = true }

#define PL_FULL(pos_x, pos_y, alt, ...) \
    (struct plane)                      \
    {                                   \
        .pos = { pos_x, pos_y },        \
        .altitude = alt,                \
        .is_active = true,              \
        .left_origin = true,            \
        __VA_ARGS__                     \
    }

#define EP(type_, pos_x, pos_y, dir_) \
    (struct endpoint) { .pos = { pos_x, pos_y }, .type = type_, .dir = dir_ }

#endif // __TEST_STATE_H__
