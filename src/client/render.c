#include "atc/plane.h"
#include "atc/state.h"
#include "atc-ncurses/render.h"
#include "atc/utils.h"
#include <ncurses.h>
#include <stdio.h>

enum atc_color_pair {
	CP_PLANE = 1,
	CP_AIRPORT,
	CP_EXIT,
	CP_BEACON,
	CP_BORDER,
	CP_NORMAL,
	CP_PATH,
};

#define CHAR_AIRPORT '^'
#define CHAR_BEACON '*'

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
	buf[0] = CHAR_AIRPORT;
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

void draw_state(struct renderer *rndr, struct state *state)
{
	curs_set(0);
	wclear(rndr->win_radar);

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

	curs_set(1);
	wrefresh(rndr->win_status);
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

	wclear(rndr->win_status);
	mvwaddstr(rndr->win_status, 0, 0, buf);
	mvwaddstr(rndr->win_status, 2, 0, "Hit space for top players list...");
	wrefresh(rndr->win_status);
}

int print_command(struct renderer *rndr, int x_offset, char *buf)
{
	if (x_offset == 0) {
		wclear(rndr->win_status);
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

void render_deinit(struct renderer *rndr)
{
	delwin(rndr->win_radar);
	delwin(rndr->win_comms);
	delwin(rndr->win_status);
	endwin();
}
