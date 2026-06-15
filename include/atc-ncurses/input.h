/** @file
 * @brief Input for issueing comms/advancing the game.
 */

#include "atc/state.h"
#include <sys/poll.h>

#define RULE_STACK_CAPACITY 15 // attb0ab0 is the longest command

struct cmd_data;
typedef const char *(*rule_func_t)(struct cmd_data *);

struct rule
{
    int token;
    int next_state;
    char *display_str;
    rule_func_t func;
};

struct rule_stack
{
    struct rule rules[RULE_STACK_CAPACITY];
    char cmd_buf[RULE_STACK_CAPACITY]; // TODO: do i null terminate this
    size_t level;
    size_t display_str_offset;
};

struct cmd_data
{
    struct rule_stack stack;
    struct state *state;
    struct plane new_plane_data;
    struct plane *state_plane;
    int cmd_line_y;
    char char_token;
    bool reset_timer;
};

struct input_data
{
    struct cmd_data cmd_data;
    struct pollfd stdin_pollfd;
    int update_interval;
};

void input_init(struct input_data *data, struct state *state);
void process_input_during_update_interval(struct input_data *data);

bool is_alpha(char ch);
bool is_capital(char ch);
bool is_numeric(char ch);
bool is_dir(char ch);
