/** @file 
 * @brief Input for issueing comms/advancing the game.
 * 
 * The input state diagram:
 * ```
 * ---> <ret>
 *  |                                                                       /
 *  -> [a-z] -> a -> +d/-c -> [0-9] -> <ret>                                 
 *           |    |                                                          
 *           |    -> [0-9] -> <ret>                                          
 *           |                                                               
 *           -> t -> <dir>/L/R -> @a/<ret> -> b -> [0-9] -> <ret>            
 *           |    |                                                          
 *           |    -> l/r -> <dir> -> @a/<ret> -> b -> [0-9] -> <ret>         
 *           |    |                                                          
 *           |    -> t -> a/e/b* -> [0-9] -> @a/<ret> -> b -> [0-9] -> <ret> 
 *           |                                                               
 *           -> c -> l/r/<ret> -> @a/<ret> -> b -> [0-9] -> <ret>            
 *           |                                                               
 *           -> m/u/i -> <ret>                                               
 * ```
 */

#include "atc-ncurses/input.h"
#include "atc-ncurses/render.h"
#include "atc/dir.h"
#include "atc/plane.h"
#include "atc/state.h"
#include "atc/utils.h"
#include <bits/time.h>
#include <ncurses.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>

#define ERR_UNDERLINE_CH '^'

#define GROUP_ALPHA 0b1
#define GROUP_NUM 0b10
#define GROUP_DIR 0b100
#define TOKEN_GROUP(group) (512 | group)
#define HAS_GROUP(token, token_group)   \
	(token < 512 || token_group < 512 ? \
		 false :                        \
		 (token & (~512)) & (token_group & (~512)))

#define TOKEN_HELP '?'
#define TOKEN_RET '\n'
#define TOKEN_BCKSPACE 127
#define TOKEN_DELWORD ('W' - 64)
#define TOKEN_DELLINE ('U' - 64)

const char *rule_advance_game(struct cmd_data *data, char)
{
	data->reset_timer = true;
	return "";
}

const char *rule_set_plane(struct cmd_data *data, char token)
{
	size_t plane_num = token - (is_capital(token) ? 'A' : 'a');
	data->state_plane = get_plane(data->state, plane_num);
	if (data->state_plane == NULL) {
		return "Unknown plane";
	}
	data->new_plane_data = *data->state_plane;
	return "";
}
const char *rule_altitude_set(struct cmd_data *data, char token)
{
	int val = char_digit_to_int(token);
	assert(0 <= val && val <= 9 && "Invalid target altitude");
	if (data->new_plane_data.target_altitude == val &&
		data->new_plane_data.altitude == val) {
		return "Already at that altitude";
	}
	data->new_plane_data.target_altitude = val;
	return "";
}
const char *rule_altitude_climb(struct cmd_data *data, char token)
{
	int val = char_digit_to_int(token);
	int target_altitude = data->new_plane_data.altitude + val;
	assert(target_altitude >= 0);
	assert(target_altitude >= data->new_plane_data.altitude);
	if (target_altitude > 9) {
		return "Altitude would be too high";
	}
	data->new_plane_data.target_altitude = target_altitude;
	return "";
}
const char *rule_altitude_descend(struct cmd_data *data, char token)
{
	int val = char_digit_to_int(token);
	int target_altitude = data->new_plane_data.altitude - val;
	assert(target_altitude <= 9);
	assert(target_altitude <= data->new_plane_data.altitude);
	if (target_altitude < 0) {
		return "Altitude would be too low";
	}
	data->new_plane_data.target_altitude = target_altitude;
	return "";
}
const char *rule_set_dir(struct cmd_data *data, char token)
{
	dir_t dir = char_to_dir(token);
	plane_comm_turn(&data->new_plane_data, dir);
	return "";
}
const char *rule_dir_left(struct cmd_data *data, char token)
{
	dir_t dir = char_to_dir(token);
	plane_comm_turn_left(&data->new_plane_data, dir);
	return "";
}
const char *rule_dir_right(struct cmd_data *data, char token)
{
	dir_t dir = char_to_dir(token);
	plane_comm_turn_right(&data->new_plane_data, dir);
	return "";
}
const char *rule_dir_left90(struct cmd_data *data, char)
{
	plane_comm_turn_left(&data->new_plane_data, DIR_90);
	return "";
}
const char *rule_dir_right90(struct cmd_data *data, char)
{
	plane_comm_turn_right(&data->new_plane_data, DIR_90);
	return "";
}
const char *rule_dir_left45(struct cmd_data *data, char)
{
	plane_comm_turn_left(&data->new_plane_data, DIR_45);
	return "";
}
const char *rule_dir_right45(struct cmd_data *data, char)
{
	plane_comm_turn_right(&data->new_plane_data, DIR_45);
	return "";
}
const char *rule_turn_towards_airport(struct cmd_data *data, char)
{
	_THROW_NOT_YET_IMPLEMENTED();
}
const char *rule_turn_towards_exit(struct cmd_data *data, char)
{
	_THROW_NOT_YET_IMPLEMENTED();
}
const char *rule_turn_towards_beacon(struct cmd_data *data, char)
{
	_THROW_NOT_YET_IMPLEMENTED();
}
const char *rule_circle_cw(struct cmd_data *data, char)
{
	plane_comm_circle(&data->new_plane_data, CDIR_CW);
	return "";
}
const char *rule_circle_ccw(struct cmd_data *data, char)
{
	plane_comm_circle(&data->new_plane_data, CDIR_CCW);
	return "";
}
const char *rule_delay_at_beacon(struct cmd_data *data, char token)
{
	size_t beacon_num = char_digit_to_int(token);
	if (beacon_num >= data->state->num_beacons) {
		return "Unknown beacon";
	}
	// TODO: check if beacon is on the flight path
	struct beacon *beacon = &data->state->beacons[beacon_num];
	data->new_plane_data.comm.at_beacon = beacon;
	return "";
}
const char *rule_mark(struct cmd_data *data, char)
{
	if (data->new_plane_data.mark == MS_MARKED) {
		return "Already marked";
	}
	data->new_plane_data.mark = MS_MARKED;
	return "";
}
const char *rule_unmark(struct cmd_data *data, char)
{
	if (data->new_plane_data.mark == MS_UNMARKED) {
		return "Already unmarked";
	}
	data->new_plane_data.mark = MS_UNMARKED;
	return "";
}
const char *rule_ignore(struct cmd_data *data, char)
{
	if (data->new_plane_data.mark == MS_IGNORED) {
		return "Already ignored";
	}
	data->new_plane_data.mark = MS_IGNORED;
	return "";
}

void format_ch(const char *fmt, char token, char *dest, size_t dest_maxlen)
{
	int ret = snprintf(dest, dest_maxlen, fmt, token);
	if (ret < 0) {
		perror("snprintf");
		assert("format_ch");
	}
	if ((size_t)ret >= dest_maxlen) {
		assert("output was trunkated");
	}
}
void format_dir(const char *fmt, char token, char *dest, size_t dest_maxlen)
{
	int ret =
		snprintf(dest, dest_maxlen, fmt, dir_to_int_angle(char_to_dir(token)));
	if (ret < 0) {
		perror("snprintf");
		assert("format_dir");
	}
	if ((size_t)ret >= dest_maxlen) {
		assert("output was trunkated");
	}
}

enum cmd_state_num {
	CMDSTATE_BEGIN,
	CMDSTATE_RETURN,
	CMDSTATE_DELAY,
	CMDSTATE_DELAY_AT,
	CMDSTATE_DELAY_AT_B,
	CMDSTATE_PL,
	CMDSTATE_PL_A,
	CMDSTATE_PL_A_C,
	CMDSTATE_PL_A_D,
	CMDSTATE_PL_T,
	CMDSTATE_PL_T_L,
	CMDSTATE_PL_T_R,
	CMDSTATE_PL_T_T,
	CMDSTATE_PL_T_T_A,
	CMDSTATE_PL_T_T_E,
	CMDSTATE_PL_T_T_B,
	CMDSTATE_PL_C,
	CMDSTATE_END = -1,
};

// TODO: TOKEN_HELP
struct rule state_begin[] = {
	{ TOKEN_GROUP(GROUP_ALPHA), CMDSTATE_PL, "%c:", rule_set_plane, format_ch },
	{ TOKEN_RET, CMDSTATE_END, "", rule_advance_game, NULL },
};
struct rule state_return[] = {
	{ TOKEN_RET, CMDSTATE_END, "", NULL, NULL },
};
struct rule state_delay[] = {
	{ 'a', CMDSTATE_DELAY_AT, " at", NULL, NULL },
	{ '@', CMDSTATE_DELAY_AT, " at", NULL, NULL },
	{ TOKEN_RET, CMDSTATE_END, "", NULL, NULL },
};
struct rule state_delay_at[] = {
	{ 'b', CMDSTATE_DELAY_AT_B, " beacon #", NULL, NULL },
	{ '*', CMDSTATE_DELAY_AT_B, " beacon #", NULL, NULL },
};
struct rule state_delay_at_b[] = {
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_RETURN, " %c", rule_delay_at_beacon,
	  format_ch },
};
struct rule state_pl[] = {
	{ 'a', CMDSTATE_PL_A, " altitude:", NULL, NULL },
	{ 't', CMDSTATE_PL_T, " turn", NULL, NULL },
	{ 'c', CMDSTATE_PL_C, " circle", NULL, NULL },
	{ 'm', CMDSTATE_RETURN, " mark", rule_mark, NULL },
	{ 'u', CMDSTATE_RETURN, " unmark", rule_unmark, NULL },
	{ 'i', CMDSTATE_RETURN, " ignore", rule_ignore, NULL },
};
struct rule state_pl_a[] = {
	{ 'c', CMDSTATE_PL_A_C, " climb", NULL, NULL },
	{ '+', CMDSTATE_PL_A_C, " climb", NULL, NULL },
	{ 'd', CMDSTATE_PL_A_D, " descend", NULL, NULL },
	{ '-', CMDSTATE_PL_A_D, " descend", NULL, NULL },
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_RETURN, " %c000 ft", rule_altitude_set,
	  format_ch },
};
struct rule state_pl_a_c[] = {
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_RETURN, " %c000 ft", rule_altitude_climb,
	  format_ch },
};
struct rule state_pl_a_d[] = {
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_RETURN, " %c000 ft",
	  rule_altitude_descend, format_ch },
};
struct rule state_pl_t[] = {
	{ TOKEN_GROUP(GROUP_DIR), CMDSTATE_DELAY, " to %d", rule_set_dir,
	  format_dir },
	{ 'L', CMDSTATE_DELAY, " left 90", rule_dir_left90, NULL },
	{ 'R', CMDSTATE_DELAY, " right 90", rule_dir_right90, NULL },
	{ 'l', CMDSTATE_PL_T_L, " left", NULL, NULL },
	{ 'r', CMDSTATE_PL_T_R, " right", NULL, NULL },
	{ 't', CMDSTATE_PL_T_T, " towards", NULL, NULL },
};
struct rule state_pl_t_l[] = {
	{ TOKEN_GROUP(GROUP_DIR), CMDSTATE_DELAY, " %d", rule_dir_left,
	  format_dir },
	{ TOKEN_RET, CMDSTATE_END, "", rule_dir_left45, NULL },
};
struct rule state_pl_t_r[] = {
	{ TOKEN_GROUP(GROUP_DIR), CMDSTATE_DELAY, " %d", rule_dir_right,
	  format_dir },
	{ TOKEN_RET, CMDSTATE_END, "", rule_dir_right45, NULL },
};
struct rule state_pl_t_t[] = {
	{ 'a', CMDSTATE_PL_T_T_A, " airport #", NULL, NULL },
	{ 'e', CMDSTATE_PL_T_T_E, " exit #", NULL, NULL },
	{ 'b', CMDSTATE_PL_T_T_B, " beacon #", NULL, NULL },
	{ '*', CMDSTATE_PL_T_T_B, " beacon #", NULL, NULL },
};
struct rule state_pl_t_t_a[] = {
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_DELAY, " %c", rule_turn_towards_airport,
	  format_ch },
};
struct rule state_pl_t_t_e[] = {
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_DELAY, " %c", rule_turn_towards_exit,
	  format_ch },
};
struct rule state_pl_t_t_b[] = {
	{ TOKEN_GROUP(GROUP_NUM), CMDSTATE_DELAY, " %c", rule_turn_towards_beacon,
	  format_ch },
};
struct rule state_pl_c[] = {
	{ 'a', CMDSTATE_DELAY_AT, " at", rule_circle_cw, NULL },
	{ '@', CMDSTATE_DELAY_AT, " at", rule_circle_cw, NULL },
	{ TOKEN_RET, CMDSTATE_END, "", rule_circle_cw, NULL },
	// These were not implemented in atc from bsdgames2 package but mentioned in the manual
	{ 'l', CMDSTATE_DELAY, " counterclockwise", rule_circle_ccw, NULL },
	{ 'r', CMDSTATE_DELAY, " clockwise", rule_circle_cw, NULL },
};

struct cmd_state {
	struct rule *rules;
	size_t len;
};

#define DEFSTATE(st) { .rules = st, .len = sizeof(st) / sizeof(st[0]) }

struct cmd_state states[] = {
	[CMDSTATE_BEGIN] = DEFSTATE(state_begin),
	[CMDSTATE_RETURN] = DEFSTATE(state_return),
	[CMDSTATE_DELAY] = DEFSTATE(state_delay),
	[CMDSTATE_DELAY_AT] = DEFSTATE(state_delay_at),
	[CMDSTATE_DELAY_AT_B] = DEFSTATE(state_delay_at_b),
	[CMDSTATE_PL] = DEFSTATE(state_pl),
	[CMDSTATE_PL_A] = DEFSTATE(state_pl_a),
	[CMDSTATE_PL_A_C] = DEFSTATE(state_pl_a_c),
	[CMDSTATE_PL_A_D] = DEFSTATE(state_pl_a_d),
	[CMDSTATE_PL_T] = DEFSTATE(state_pl_t),
	[CMDSTATE_PL_T_L] = DEFSTATE(state_pl_t_l),
	[CMDSTATE_PL_T_R] = DEFSTATE(state_pl_t_r),
	[CMDSTATE_PL_T_T] = DEFSTATE(state_pl_t_t),
	[CMDSTATE_PL_T_T_A] = DEFSTATE(state_pl_t_t_a),
	[CMDSTATE_PL_T_T_E] = DEFSTATE(state_pl_t_t_e),
	[CMDSTATE_PL_T_T_B] = DEFSTATE(state_pl_t_t_b),
	[CMDSTATE_PL_C] = DEFSTATE(state_pl_c),
};

void input_init(struct input_data *data, struct state *state,
				struct renderer *renderer)
{
	// timeout(0);
	data->renderer = renderer;
	data->cmd_data.state = state;
	data->stdin_pollfd =
		(struct pollfd){ .fd = STDIN_FILENO, .events = POLLIN };
	data->cmd_data.stack.rules[0] =
		(struct rule){ -1, CMDSTATE_BEGIN, "", NULL, NULL };
	data->cmd_data.stack.str_offsets[0] = 0;
	data->cmd_data.stack.level = 0;
	data->update_interval = state->update_interval;
}

int get_token_group(int ch)
{
	int group = 0;
	if (is_alpha(ch)) {
		group |= GROUP_ALPHA;
	}
	if (is_numeric(ch)) {
		group |= GROUP_NUM;
	}
	if (is_dir(ch)) {
		group |= GROUP_DIR;
	}
	if (group != 0) {
		return TOKEN_GROUP(group);
	}
	return ch;
}

void exec_cmd(struct renderer *rndr, struct cmd_data *args)
{
	args->reset_timer = false;
	args->stack.level = 0;

	struct rule rule = {};
	const char *err_msg = NULL;
	int err_rule_idx = -1;
	bool break_on_next = false;
	for (int i = 0; !break_on_next; i++) {
		assert(i < RULE_STACK_CAPACITY);
		rule = args->stack.rules[i];
		if (rule.next_state == CMDSTATE_END) {
			break_on_next = true;
		}
		if (rule.func == NULL) {
			continue;
		}
		char char_token = args->stack.cmd_buf[i];
		err_msg = rule.func(args, char_token);
		if (strcmp(err_msg, "") != 0) {
			err_rule_idx = i;
			break;
		}
	}
	if (err_rule_idx == 0) {
		assert("BEGIN rule returned an error");
		return;
	}
	if (err_rule_idx != -1) {
		int end = args->stack.str_offsets[err_rule_idx];
		int len = end - args->stack.str_offsets[err_rule_idx - 1];
		print_cmd_err(rndr, end - len, len, err_msg);
		return;
	}
	assert((args->reset_timer == true || args->state_plane != NULL) &&
		   "Command requires a plane to be set but it's NULL");
	if (!args->reset_timer) {
		memcpy(args->state_plane, &args->new_plane_data, sizeof(struct plane));
	}
    cmd_reset_cursor(rndr);
}

bool handle_del_chars(int ch, struct input_data *data)
{
	struct rule_stack *stack = &data->cmd_data.stack;
	if (stack->level == 0) {
		return false;
	}

	bool delline = false;
	switch (ch) {
	case KEY_BACKSPACE:
	case '\b':
	case TOKEN_BCKSPACE:
	case TOKEN_DELWORD:
		break;
	case TOKEN_DELLINE:
		delline = true;
		break;
	default:
		return false;
	}

	size_t prev_offset = stack->str_offsets[stack->level];
	stack->level = delline ? 0 : (stack->level - 1);
	size_t curr_offset = stack->str_offsets[stack->level];
	assert(prev_offset >= curr_offset);
	clear_command(data->renderer, curr_offset, prev_offset - curr_offset);

	return true;
}

void process_input_during_update_interval(struct input_data *data)
{
	int remaining_ms = data->update_interval * 1000;
	while (remaining_ms > 0) {
		struct timespec before_poll = {};
		clock_gettime(CLOCK_MONOTONIC, &before_poll);

		int poll_val = poll(&data->stdin_pollfd, 1, remaining_ms);

		struct timespec after_poll = {};
		clock_gettime(CLOCK_MONOTONIC, &after_poll);

		int waited_ms = (after_poll.tv_sec - before_poll.tv_sec) * 1000 +
						(after_poll.tv_nsec - before_poll.tv_nsec) / 1e6;
		remaining_ms -= waited_ms;
		if (poll_val < 0) {
			perror("poll");
			continue;
		}
		if (!(data->stdin_pollfd.revents & POLLIN))
			continue;

		int ch = getch();
		if (ch == ERR) {
			continue;
		}
		if (handle_del_chars(ch, data))
			continue;

		int token_gr = get_token_group(ch);

		struct rule_stack *stack = &data->cmd_data.stack;
		size_t state_num = stack->rules[stack->level].next_state;
		struct cmd_state state = states[state_num];
		struct rule rule = {};
		bool found = false;
		for (size_t i = 0; i < state.len; i++) {
			rule = state.rules[i];
			if (HAS_GROUP(token_gr, rule.token) || rule.token == ch) {
				found = true;
				break;
			}
		}
		if (!found)
			continue;

		char st[30];
		if (rule.fmt_func != NULL) {
			rule.fmt_func(rule.str_fmt, ch, st, 30);
		} else {
			strncpy(st, rule.str_fmt, 30);
		}
		int next_offset =
			print_command(data->renderer, stack->str_offsets[stack->level], st);

		stack->level++;
		stack->str_offsets[stack->level] = next_offset;
		stack->rules[stack->level] = rule;
		stack->cmd_buf[stack->level] = ch;

		if (rule.next_state == CMDSTATE_END) {
			exec_cmd(data->renderer, &data->cmd_data);
			if (data->cmd_data.reset_timer) {
				break;
			}
		}
	}
}
