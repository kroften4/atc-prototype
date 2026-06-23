#include "atc/plane.h"
#include "atc/state.h"
#include "atc-ncurses/render.h"
#include "atc/utils.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

enum atc_color_pair {
	CP_PLANE = 1,
	CP_AIRPORT,
	CP_EXIT,
	CP_BEACON,
	CP_BORDER,
	CP_NORMAL,
	CP_PATH,
};

#define CHAR_BEACON '*'

#define STR_AIRPORT_DIRS "^,>`v'<."
#ifdef UNICODE
static const char *unicode_airort_dirs[] = {
	"^", "↗", ">", "↘", "v", "↙", "<", "↖",
};
#endif

#define CHAR_ERR_UNDERLINE '^'

struct renderer {
	WINDOW *win_radar;
	WINDOW *win_comms;
	WINDOW *win_status;
};

#define STATUS_LINES 3
#define COMMS_COLS 20

void render_init(struct renderer *rndr)
{
	initscr();
	cbreak();
	noecho();

	if (has_colors())
		start_color();

	init_pair(CP_PLANE, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_AIRPORT, COLOR_RED, COLOR_BLACK);
	init_pair(CP_EXIT, COLOR_GREEN, COLOR_BLACK);
	init_pair(CP_BEACON, COLOR_CYAN, COLOR_BLACK);
	init_pair(CP_BORDER, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_NORMAL, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_PATH, COLOR_WHITE, COLOR_BLACK);

	// curs_set(0);

	rndr->win_radar = newwin(LINES - STATUS_LINES, COLS - COMMS_COLS, 0, 0);
	rndr->win_comms = newwin(0, 0, 0, COLS - COMMS_COLS);
	rndr->win_status = newwin(0, COLS - COMMS_COLS, LINES - STATUS_LINES, 0);
}

char plane_letter_from_idx(struct plane *plane, size_t plane_idx)
{
	switch (plane->type) {
	case PLANE_JET:
		return 'a' + plane_idx;
	case PLANE_PROP:
		return 'A' + plane_idx;
	default:
		return '\0';
	}
}

void draw_plane(struct renderer *rndr, struct state *state, size_t plane_idx,
				char buf[3])
{
	assert(plane_idx < state->num_planes);
	struct plane *plane = &state->planes[plane_idx];
	if (!plane->is_active || plane->comm.type == COMM_HOLD) {
		return;
	}
	if (plane->mark == MS_MARKED) {
		wattrset(rndr->win_radar, COLOR_PAIR(CP_PLANE) | A_REVERSE);
	} else {
		wattrset(rndr->win_radar, COLOR_PAIR(CP_PLANE));
	}

	buf[0] = plane_letter_from_idx(plane, plane_idx);
	buf[1] = int_digit_to_char(plane->altitude);
	buf[2] = '\0';
	mvwaddstr(rndr->win_radar, plane->pos.y, plane->pos.x * 2, buf);
}

void draw_airport(struct renderer *rndr, struct endpoint *airport, char buf[3])
{
	wattrset(rndr->win_radar, COLOR_PAIR(CP_AIRPORT));
	buf[0] = STR_AIRPORT_DIRS[airport->dir];
	buf[1] = int_digit_to_char(airport->num);
	buf[2] = '\0';
	mvwaddstr(rndr->win_radar, airport->pos.y, airport->pos.x * 2, buf);
}

void draw_exit(struct renderer *rndr, struct endpoint *ext, char buf[3])
{
	wattrset(rndr->win_radar, COLOR_PAIR(CP_EXIT));
	buf[0] = int_digit_to_char(ext->num);
	buf[1] = '\0';
	mvwaddstr(rndr->win_radar, ext->pos.y, ext->pos.x * 2, buf);
}

void draw_beacon(struct renderer *rndr, struct beacon *beacon, char buf[3])
{
	wattrset(rndr->win_radar, COLOR_PAIR(CP_BEACON));
	buf[0] = CHAR_BEACON;
	buf[1] = int_digit_to_char(beacon->num);
	buf[2] = '\0';
	mvwaddstr(rndr->win_radar, beacon->pos.y, beacon->pos.x * 2, buf);
}

void draw_endpoint(struct renderer *rndr, struct endpoint *ep, char buf[3])
{
	switch (ep->type) {
	case EP_AIRPORT:
		draw_airport(rndr, ep, buf);
		break;
	case EP_EXIT:
		draw_exit(rndr, ep, buf);
		break;
	}
}

void print_game_over(struct renderer *rndr, struct state *state,
					 struct flight_end_data fle_data)
{
	char buf[60];
	struct plane plane = state->planes[fle_data.plane_idx];
	char pl_ch = get_plane_char(fle_data.plane_idx, plane.type);
	switch (fle_data.type) {
	case FLE_COLLISION:
		struct plane plane2 = state->planes[fle_data.data.coll_plane_idx];
		char pl2_ch = get_plane_char(fle_data.data.coll_plane_idx, plane2.type);
		(void)snprintf(buf, sizeof(buf), "Plane '%c' collided with plane '%c'.",
					   pl_ch, pl2_ch);
		break;
	case FLE_CRASH:
		(void)snprintf(buf, sizeof(buf), "Plane '%c' crashed on the ground.",
					   pl_ch);
		break;
	case FLE_SUCCESS:
		(void)snprintf(buf, sizeof(buf),
					   "Plane '%c' landed successfully. Why did the game end?",
					   pl_ch);
		break;
	case FLE_EXIT_NOT_LAND:
		(void)snprintf(buf, sizeof(buf), "Plane '%c' exited instead of landed.",
					   pl_ch);
		break;
	case FLE_ILLEGAL_EXIT:
		(void)snprintf(buf, sizeof(buf),
					   "Plane '%c' illegally left the flight arena.", pl_ch);
		break;
	case FLE_LAND_NOT_EXIT:
		(void)snprintf(buf, sizeof(buf), "Plane '%c' landed instead of exited.",
					   pl_ch);
		break;
	case FLE_WRONG_AIRPORT:
		(void)snprintf(buf, sizeof(buf),
					   "Plane '%c' landed at the wrong airport.", pl_ch);
		break;
	case FLE_WRONG_EXIT:
		(void)snprintf(buf, sizeof(buf),
					   "Plane '%c' exited via the wrong exit.", pl_ch);
		break;
	case FLE_IN_PROCESS:
		(void)snprintf(
			buf, sizeof(buf),
			"Plane '%c' flight did not finish. Why did the game end?", pl_ch);
		break;
	case FLE_OUT_OF_FUEL:
		(void)snprintf(buf, sizeof(buf), "Plane '%c' ran out of fuel.", pl_ch);
		break;
	case FLE_LAND_WRONG_DIR:
		(void)snprintf(buf, sizeof(buf),
					   "Plane '%c' landed in the wrong direction.", pl_ch);
		break;
	case FLE_WRONG_ALTITUDE:
		(void)snprintf(buf, sizeof(buf),
					   "Plane '%c' exited at the wrong altitude.", pl_ch);
		break;
	}

	werase(rndr->win_status);
	mvwaddstr(rndr->win_status, 0, 0, buf);
	mvwaddstr(rndr->win_status, 2, 0, "Hit space for top players list...");
	wrefresh(rndr->win_status);
}

int print_command(struct renderer *rndr, int x_offset, char *buf)
{
	if (x_offset == 0) {
		werase(rndr->win_status);
	}
	mvwaddstr(rndr->win_status, 0, x_offset, buf);
	wrefresh(rndr->win_status);
	return getcurx(rndr->win_status);
}

void clear_command(struct renderer *rndr, int x_offset, size_t len)
{
	wmove(rndr->win_status, 0, x_offset);
	for (size_t x = x_offset; x < x_offset + len; x++) {
		waddch(rndr->win_status, ' ');
	}
	wmove(rndr->win_status, 0, x_offset);
	wrefresh(rndr->win_status);
}

void print_cmd_err(struct renderer *rndr, int underline_x_offset,
				   int underline_len, const char *err_msg_buf)
{
	wmove(rndr->win_status, 1, underline_x_offset);
	for (int x = underline_x_offset; x < underline_x_offset + underline_len;
		 x++) {
		waddch(rndr->win_status, CHAR_ERR_UNDERLINE);
	}
	mvwaddstr(rndr->win_status, 2, 0, err_msg_buf);
	wmove(rndr->win_status, 0, 0);
	wrefresh(rndr->win_status);
}

void cmd_reset_cursor(struct renderer *rndr)
{
	wmove(rndr->win_status, 0, 0);
	wrefresh(rndr->win_status);
}

void draw_time_str(struct renderer *rndr, struct state *state, char *buf)
{
	if (snprintf(buf, COMMS_COLS, "Time: %-4zu Safe: %-2zu", state->time,
				 state->planes_safe) < 0) {
		perror("snprintf");
	}
	mvwaddstr(rndr->win_comms, 0, 0, buf);
}

void draw_plane_comm(struct renderer *rndr, struct plane *pl, char *buf,
					 int y_pos)
{
	char pl_dt_str[] = {
		plane_letter_from_idx(pl, pl->num),
		int_digit_to_char(pl->altitude),
		pl->fuel < PLANE_LOW_FUEL_MARK ? '*' : ' ',
		pl->destination->type == EP_EXIT ? 'E' : 'A',
		int_digit_to_char(pl->destination->num),
		':',
		' ',
		'\0',
	};
	strcpy(buf, pl_dt_str);

	int ret = 0;
	switch (pl->comm.type) {
	case COMM_HOLD:
		// TODO: make it print the airport
		ret = snprintf(buf + sizeof(pl_dt_str) - 1, COMMS_COLS, "Holding @ A%c",
					   '?');
		break;
	case COMM_TURN:
		ret = snprintf(buf + sizeof(pl_dt_str) - 1, COMMS_COLS, "%d",
					   dir_to_int_angle(pl->comm.data.target_dir));
		break;
	case COMM_CIRCLE:
		if (pl->comm.data.circle_dir == CDIR_CCW) {
			strcat(buf, "Cir CCW");
		} else {
			strcat(buf, "Circle");
		}
		break;
	case COMM_NONE:
		if (pl->mark != MS_MARKED) {
			strcat(buf, "---------");
		}
		break;
	}
	if (ret < 0) {
		perror("snprintf");
		return;
	}

	if (pl->comm.at_beacon != NULL) {
		char at_beacon_str[] = {
			' ',  '@', ' ', 'B', int_digit_to_char(pl->comm.at_beacon->num),
			'\0',
		};
		assert(strlen(buf) + strlen(at_beacon_str) < COMMS_COLS);
		strcat(buf, at_beacon_str);
	}

	mvwaddstr(rndr->win_comms, y_pos, 0, buf);
}

void render_comms(struct renderer *rndr, struct state *state)
{
	werase(rndr->win_comms);

	char buf[COMMS_COLS] = "";
	draw_time_str(rndr, state, buf);
	mvwaddstr(rndr->win_comms, 2, 0, "pl dt  comm");

	struct plane *plane = NULL;
	size_t curr_y_pos = 3;
	for (size_t i = 0; i < state->num_planes; i++) {
		plane = &state->planes[i];
		if (!plane->is_active) {
			continue;
		}
		if (plane->comm.type == COMM_HOLD) {
			continue;
		}
		draw_plane_comm(rndr, plane, buf, curr_y_pos);
		curr_y_pos++;
	}
	curr_y_pos++;

	for (size_t i = 0; i < state->num_planes; i++) {
		plane = &state->planes[i];
		if (!plane->is_active) {
			continue;
		}
		if (plane->comm.type != COMM_HOLD) {
			continue;
		}
		draw_plane_comm(rndr, plane, buf, curr_y_pos);
		curr_y_pos++;
	}

	wrefresh(rndr->win_comms);
}

void draw_path(struct renderer *rndr, struct path *path)
{
	int x = path->start.x;
	int y = path->start.y;
	while (true) {
		mvwaddch(rndr->win_radar, y, x * 2, '+');
		if (x == path->end.x && y == path->end.y) {
			break;
		}
		if (x < path->end.x) {
			x++;
		} else if (x > path->end.x) {
			x--;
		}
		if (y < path->end.y) {
			y++;
		} else if (y > path->end.y) {
			y--;
		}
	}
}

void draw_state(struct renderer *rndr, struct state *state)
{
	curs_set(0);
	werase(rndr->win_radar);

	for (int y = 0; y < state->bounds.y; y++) {
		wmove(rndr->win_radar, y, 0);
		for (int x = 0; x < state->bounds.x; x++) {
			waddstr(rndr->win_radar, ". ");
		}
	}

	for (int y = 0; y < state->bounds.y; y++) {
		mvwaddch(rndr->win_radar, y, 0, '|');
	}
	for (int y = 0; y < state->bounds.y; y++) {
		mvwaddch(rndr->win_radar, y, (state->bounds.x - 1) * 2, '|');
	}

	wmove(rndr->win_radar, 0, 0);
	for (int x = 0; x < state->bounds.x * 2 - 1; x++) {
		waddch(rndr->win_radar, '-');
	}
	wmove(rndr->win_radar, state->bounds.y - 1, 0);
	for (int x = 0; x < state->bounds.x * 2 - 1; x++) {
		waddch(rndr->win_radar, '-');
	}

	for (size_t i = 0; i < state->num_paths; i++) {
		struct path *path = &state->paths[i];
		draw_path(rndr, path);
	}

	char buf[] = "A0";
	for (size_t i = 0; i < state->num_beacons; i++) {
		struct beacon *beacon = &state->beacons[i];
		draw_beacon(rndr, beacon, buf);
	}

	for (size_t i = 0; i < state->num_endpoints; i++) {
		struct endpoint *ep = &state->endpoints[i];
		draw_endpoint(rndr, ep, buf);
	}

	for (size_t i = 0; i < state->num_planes; i++) {
		draw_plane(rndr, state, i, buf);
	}
	wattrset(rndr->win_radar, COLOR_PAIR(CP_NORMAL));
	wrefresh(rndr->win_radar);

	render_comms(rndr, state);

	curs_set(1);
	wrefresh(rndr->win_status);
}

void render_deinit(struct renderer *rndr)
{
	delwin(rndr->win_radar);
	delwin(rndr->win_comms);
	delwin(rndr->win_status);
	noraw();
	endwin();
}
