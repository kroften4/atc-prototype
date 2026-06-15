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

#define ERR_RULE_UNDERLINE_STR "^^^^^^^^^^^^^^^^^^^^"

#define TOKEN_LETTER 512
#define TOKEN_NUM 513
#define TOKEN_DIR 514
#define TOKEN_HELP '?'
#define TOKEN_RET '\n'
#define TOKEN_DELWORD ('W' - 64)
#define TOKEN_DELLINE ('U' - 64)

int char_digit_to_int(char digit)
{
	return digit - '0';
}

const char *rule_advance_game(struct cmd_data *data)
{
	data->reset_timer = true;
	return "";
}

const char *rule_set_plane(struct cmd_data *data)
{
	size_t plane_num =
		data->char_token - (is_capital(data->char_token) ? 'A' : 'a');
	data->state_plane = get_plane(data->state, plane_num);
	if (data->state_plane == NULL) {
		return "Unknown plane";
	}
	data->new_plane_data = *data->state_plane;
	return "";
}
const char *rule_altitude_set(struct cmd_data *data)
{
	int val = char_digit_to_int(data->char_token);
	assert(0 <= val && val <= 9 && "Invalid target altitude");
	if (data->new_plane_data.target_altitude == val &&
		data->new_plane_data.altitude == val) {
		return "Already at that altitude";
	}
	data->new_plane_data.target_altitude = val;
	return "";
}
const char *rule_altitude_climb(struct cmd_data *data)
{
	int val = char_digit_to_int(data->char_token);
	int target_altitude = data->new_plane_data.altitude + val;
	assert(target_altitude >= 0);
	assert(target_altitude >= data->new_plane_data.altitude);
	if (target_altitude > 9) {
		return "Altitude would be too high";
	}
	data->new_plane_data.target_altitude = val;
	return "";
}
const char *rule_altitude_descend(struct cmd_data *data)
{
	int val = char_digit_to_int(data->char_token);
	int target_altitude = data->new_plane_data.altitude - val;
	assert(target_altitude <= 9);
	assert(target_altitude <= data->new_plane_data.altitude);
	if (target_altitude < 0) {
		return "Altitude would be too low";
	}
	data->new_plane_data.target_altitude = val;
	return "";
}
const char *rule_set_dir(struct cmd_data *data)
{
	dir_t dir = char_to_dir(data->char_token);
	plane_comm_turn(&data->new_plane_data, dir);
	return "";
}
const char *rule_dir_left(struct cmd_data *data)
{
	dir_t dir = char_to_dir(data->char_token);
	plane_comm_turn_left(&data->new_plane_data, dir);
	return "";
}
const char *rule_dir_right(struct cmd_data *data)
{
	dir_t dir = char_to_dir(data->char_token);
	plane_comm_turn_right(&data->new_plane_data, dir);
	return "";
}
const char *rule_dir_left90(struct cmd_data *data)
{
	plane_comm_turn_left(&data->new_plane_data, DIR_90);
	return "";
}
const char *rule_dir_right90(struct cmd_data *data)
{
	plane_comm_turn_right(&data->new_plane_data, DIR_90);
	return "";
}
const char *rule_turn_towards_airport(struct cmd_data *data)
{
	_THROW_NOT_YET_IMPLEMENTED();
}
const char *rule_turn_towards_exit(struct cmd_data *data)
{
	_THROW_NOT_YET_IMPLEMENTED();
}
const char *rule_turn_towards_beacon(struct cmd_data *data)
{
	_THROW_NOT_YET_IMPLEMENTED();
}
const char *rule_circle_cw(struct cmd_data *data)
{
	plane_comm_circle(&data->new_plane_data, CDIR_CW);
	return "";
}
const char *rule_circle_ccw(struct cmd_data *data)
{
	plane_comm_circle(&data->new_plane_data, CDIR_CCW);
	return "";
}
const char *rule_delay_at_beacon(struct cmd_data *data)
{
	size_t beacon_num = char_digit_to_int(data->char_token);
	if (beacon_num <= data->state->num_beacons) {
		return "Unknown beacon";
	}
	// TODO: check if beacon is on the flight path
	struct beacon *beacon = &data->state->beacons[beacon_num];
	data->new_plane_data.comm.at_beacon = beacon;
	return "";
}
const char *rule_mark(struct cmd_data *data)
{
	if (data->new_plane_data.mark == MS_MARKED) {
		return "Already marked";
	}
	data->new_plane_data.mark = MS_MARKED;
	return "";
}
const char *rule_unmark(struct cmd_data *data)
{
	if (data->new_plane_data.mark == MS_UNMARKED) {
		return "Already unmarked";
	}
	data->new_plane_data.mark = MS_MARKED;
	return "";
}
const char *rule_ignore(struct cmd_data *data)
{
	if (data->new_plane_data.mark == MS_IGNORED) {
		return "Already ignored";
	}
	data->new_plane_data.mark = MS_IGNORED;
	return "";
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
	{ TOKEN_LETTER, CMDSTATE_PL, "%c:", rule_set_plane },
	{ TOKEN_RET, CMDSTATE_END, "", rule_advance_game },
};
struct rule state_return[] = {
	{ TOKEN_RET, CMDSTATE_END, "", NULL },
};
struct rule state_delay[] = {
	{ 'a', CMDSTATE_DELAY_AT, " at", NULL },
	{ '@', CMDSTATE_DELAY_AT, " at", NULL },
	{ TOKEN_RET, CMDSTATE_END, "", NULL },
};
struct rule state_delay_at[] = {
	{ 'b', CMDSTATE_DELAY_AT_B, " beacon #", NULL },
	{ '*', CMDSTATE_DELAY_AT_B, " beacon #", NULL },
};
struct rule state_delay_at_b[] = {
	{ TOKEN_NUM, CMDSTATE_RETURN, " %c", rule_delay_at_beacon },
};
struct rule state_pl[] = {
	{ 'a', CMDSTATE_PL_A, " altitude:", NULL },
	{ 't', CMDSTATE_PL_T, " turn", NULL },
	{ 'c', CMDSTATE_PL_C, " circle", NULL },
	{ 'm', CMDSTATE_RETURN, " mark", NULL },
	{ 'u', CMDSTATE_RETURN, " unmark", NULL },
	{ 'i', CMDSTATE_RETURN, " ignore", NULL },
};
struct rule state_pl_a[] = {
	{ 'c', CMDSTATE_PL_A_C, " climb", NULL },
	{ '+', CMDSTATE_PL_A_C, " climb", NULL },
	{ 'd', CMDSTATE_PL_A_D, " descend", NULL },
	{ '-', CMDSTATE_PL_A_D, " descend", NULL },
	{ TOKEN_NUM, CMDSTATE_RETURN, " %c000 ft", rule_altitude_set },
};
struct rule state_pl_a_c[] = {
	{ TOKEN_NUM, CMDSTATE_RETURN, " %c000 ft", rule_altitude_climb },
};
struct rule state_pl_a_d[] = {
	{ TOKEN_NUM, CMDSTATE_RETURN, " %c000 ft", rule_altitude_descend },
};
struct rule state_pl_t[] = {
	// { 'w', CMDSTATE_DELAY, " to 0", NULL },
	// { 'e', CMDSTATE_DELAY, " to 45", NULL },
	// { 'd', CMDSTATE_DELAY, " to 90", NULL },
	// { 'c', CMDSTATE_DELAY, " to 135", NULL },
	// { 'x', CMDSTATE_DELAY, " to 180", NULL },
	// { 'z', CMDSTATE_DELAY, " to 225", NULL },
	// { 'a', CMDSTATE_DELAY, " to 270", NULL },
	// { 'q', CMDSTATE_DELAY, " to 315", NULL },
	{ TOKEN_DIR, CMDSTATE_DELAY, " to %d", rule_set_dir },
	{ 'L', CMDSTATE_DELAY, " left 90", rule_dir_left90 },
	{ 'R', CMDSTATE_DELAY, " right 90", rule_dir_right90 },
	{ 'l', CMDSTATE_PL_T_L, " left", NULL },
	{ 'r', CMDSTATE_PL_T_R, " right", NULL },
	{ 't', CMDSTATE_PL_T_T, " towards", NULL },
};
struct rule state_pl_t_l[] = {
	// { 'w', CMDSTATE_DELAY, " 0", NULL },
	// { 'e', CMDSTATE_DELAY, " 45", NULL },
	// { 'd', CMDSTATE_DELAY, " 90", NULL },
	// { 'c', CMDSTATE_DELAY, " 135", NULL },
	// { 'x', CMDSTATE_DELAY, " 180", NULL },
	// { 'z', CMDSTATE_DELAY, " 225", NULL },
	// { 'a', CMDSTATE_DELAY, " 270", NULL },
	// { 'q', CMDSTATE_DELAY, " 315", NULL },
	{ TOKEN_DIR, CMDSTATE_DELAY, " %d", rule_dir_left },
};
struct rule state_pl_t_r[] = {
	// { 'w', CMDSTATE_DELAY, " 0", NULL },
	// { 'e', CMDSTATE_DELAY, " 45", NULL },
	// { 'd', CMDSTATE_DELAY, " 90", NULL },
	// { 'c', CMDSTATE_DELAY, " 135", NULL },
	// { 'x', CMDSTATE_DELAY, " 180", NULL },
	// { 'z', CMDSTATE_DELAY, " 225", NULL },
	// { 'a', CMDSTATE_DELAY, " 270", NULL },
	// { 'q', CMDSTATE_DELAY, " 315", NULL },
	{ TOKEN_DIR, CMDSTATE_DELAY, " %d", rule_dir_right },
};
struct rule state_pl_t_t[] = {
	{ 'a', CMDSTATE_PL_T_T_A, " airport #", NULL },
	{ 'e', CMDSTATE_PL_T_T_E, " exit #", NULL },
	{ 'b', CMDSTATE_PL_T_T_B, " beacon #", NULL },
	{ '*', CMDSTATE_PL_T_T_B, " beacon #", NULL },
};
struct rule state_pl_t_t_a[] = {
	{ TOKEN_NUM, CMDSTATE_DELAY, " %c", rule_turn_towards_airport },
};
struct rule state_pl_t_t_e[] = {
	{ TOKEN_NUM, CMDSTATE_DELAY, " %c", rule_turn_towards_exit },
};
struct rule state_pl_t_t_b[] = {
	{ TOKEN_NUM, CMDSTATE_DELAY, " %c", rule_turn_towards_beacon },
};
struct rule state_pl_c[] = {
	{ 'a', CMDSTATE_DELAY_AT, " at", NULL },
	{ '@', CMDSTATE_DELAY_AT, " at", NULL },
	{ TOKEN_RET, CMDSTATE_END, "", rule_circle_cw },
	// These were not implemented in atc from bsdgames2 package but mentioned in the manual
	{ 'l', CMDSTATE_DELAY, " counterclockwise", rule_circle_ccw },
	{ 'r', CMDSTATE_DELAY, " clockwise", rule_circle_cw },
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

void input_init(struct input_data *data, struct state *state)
{
	// timeout(0);
	data->cmd_data.state = state;
	data->stdin_pollfd =
		(struct pollfd){ .fd = STDIN_FILENO, .events = POLLIN };
	data->cmd_data.stack.rules[0] =
		(struct rule){ -1, CMDSTATE_BEGIN, "", NULL };
	data->cmd_data.stack.level = 0;
	data->cmd_data.stack.display_str_offset = 0;
	data->cmd_data.cmd_line_y = state->bounds.y + 1;
	data->update_interval = state->update_interval;
}

bool is_alpha(char ch)
{
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}
bool is_capital(char ch)
{
	return ('A' <= ch && ch <= 'Z');
}
bool is_numeric(char ch)
{
	return '0' <= ch && ch <= '9';
}
bool is_dir(char ch)
{
	return ch == 'q' || ch == 'w' || ch == 'e' || ch == 'd' || ch == 'c' ||
		   ch == 'x' || ch == 'z' || ch == 'a';
}

int get_token(int ch)
{
	if (is_alpha(ch)) {
		return TOKEN_LETTER;
	}
	if (is_numeric(ch)) {
		return TOKEN_NUM;
	}
	if (is_dir(ch)) {
		return TOKEN_DIR;
	}
	return ch;
}

void exec_cmd(struct cmd_data *args)
{
	// TODO: clear previous messages on screen
	args->reset_timer = false;
	args->stack.display_str_offset = 0;
	args->stack.level = 0;

	struct rule rule = {};
	const char *err_msg = NULL;
	int err_rule_idx = -1;
	bool break_on_next = false;
	for (int i = 0; !break_on_next; i++) {
		if (rule.next_state == CMDSTATE_END) {
			break_on_next = true;
		}
		assert(i < RULE_STACK_CAPACITY);
		rule = args->stack.rules[i];
		if (rule.func == NULL) {
			continue;
		}
		args->char_token = args->stack.cmd_buf[i];
		err_msg = rule.func(args);
		if (strcmp(err_msg, "") != 0) {
			err_rule_idx = i;
			break;
		}
	}
	if (err_rule_idx != -1) {
		size_t err_rule_len =
			strlen(args->stack.rules[err_rule_idx].display_str);
		assert(err_rule_len < sizeof(ERR_RULE_UNDERLINE_STR));
		mvaddnstr(args->cmd_line_y + 1,
				  args->stack.display_str_offset - err_rule_len,
				  ERR_RULE_UNDERLINE_STR, err_rule_len);
		mvaddstr(args->cmd_line_y + 2, 0, err_msg);
		return;
	}
	assert((args->reset_timer == true || args->state_plane != NULL) &&
		   "Command requires a plane to be set but it's NULL");
	if (!args->reset_timer) {
		memcpy(args->state_plane, &args->new_plane_data, sizeof(struct plane));
	}
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
		int token = get_token(ch);
		// TODO: handle special chars
		switch (token) {
		case KEY_BACKSPACE:
		case TOKEN_DELWORD:
			break;
		case TOKEN_DELLINE:
			break;
		}

		size_t state_num =
			data->cmd_data.stack.rules[data->cmd_data.stack.level].next_state;
		struct cmd_state state = states[state_num];
		struct rule rule = {};
		bool found = false;
		for (size_t i = 0; i < state.len; i++) {
			rule = state.rules[i];
			// TODO: this is quite awkward idk
			if (rule.token == token || rule.token == ch) {
				found = true;
				break;
			}
		}
		if (!found)
			continue;

		data->cmd_data.stack.level++;
		data->cmd_data.stack.rules[data->cmd_data.stack.level] = rule;
		data->cmd_data.stack.cmd_buf[data->cmd_data.stack.level] = ch;
		//TODO: snprinf ch
		mvaddstr(data->cmd_data.cmd_line_y,
				 data->cmd_data.stack.display_str_offset, rule.display_str);
		data->cmd_data.stack.display_str_offset = getcurx(stdscr);
		if (rule.next_state == CMDSTATE_END) {
			exec_cmd(&data->cmd_data);
			if (data->cmd_data.reset_timer) {
				break;
			}
		}
	}
}
