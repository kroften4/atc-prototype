/** @file
 * @brief Input for issueing comms/advancing the game.
 */

#include "atc-ncurses/render.h"
#include "atc/state.h"
#include <sys/poll.h>

#define RULE_STACK_CAPACITY 15 // attb0ab0 is the longest command

struct cmd_data;
typedef const char *(*rule_func_t)(struct cmd_data *, char token);
typedef void (*rule_print_format_func_t)(const char *fmt, char token,
                                         char *dest, size_t dest_maxlen);

struct rule
{
    int token;
    int next_state;
    const char *str_fmt;
    rule_func_t func;
    rule_print_format_func_t fmt_func;
};

struct rule_stack
{
    struct rule rules[RULE_STACK_CAPACITY];
    char cmd_buf[RULE_STACK_CAPACITY];
    size_t str_offsets[RULE_STACK_CAPACITY];
    size_t level;
};

struct cmd_data
{
    struct rule_stack stack;
    struct state *state;
    struct plane new_plane_data;
    struct plane *state_plane;
    bool reset_timer;
};

struct input_data
{
    struct renderer *renderer;
    struct cmd_data cmd_data;
    struct pollfd stdin_pollfd;
    int update_interval;
};

void input_init(struct input_data *data, struct state *state,
                struct renderer *renderer);
void process_input_during_update_interval(struct input_data *data);
