#include "atc/plane.h"
#include "atc/state.h"
#include "atc-ncurses/render.h"
#include <ncurses.h>
#include <stdio.h>

#define CP_PLANE 1
#define CP_PLANE_IGNORED 1
#define CP_AIRPORT 2
#define CP_EXIT 3
#define CP_BEACON 4
#define CP_BORDER 5
#define CP_NORMAL 6
#define CP_PATH 7

#define CHAR_AIRPORT "^"
#define CHAR_BEACON "*"

void render_init()
{
	initscr();
	cbreak();
	noecho();

	if (has_colors())
		start_color();

	init_pair(CP_PLANE, COLOR_BLACK, COLOR_WHITE);
	init_pair(CP_PLANE_IGNORED, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_AIRPORT, COLOR_RED, COLOR_BLACK);
	init_pair(CP_EXIT, COLOR_GREEN, COLOR_BLACK);
	init_pair(CP_BEACON, COLOR_CYAN, COLOR_BLACK);
	init_pair(CP_BORDER, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_NORMAL, COLOR_WHITE, COLOR_BLACK);
	init_pair(CP_PATH, COLOR_WHITE, COLOR_BLACK);

	curs_set(0);
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

void draw_plane(struct state *state, size_t plane_idx, char *buf)
{
	assert(plane_idx < state->num_planes);
	struct plane *plane = &state->planes[plane_idx];
	if (!plane->is_active || plane->comm.type == COMM_HOLD) {
		return;
	}

	if (snprintf(buf, 3, "%c%u", plane_letter_from_idx(plane, plane_idx),
				 plane->altitude) < 0) {
		perror("draw_plane: snprintf");
	}
	mvaddstr(plane->pos.y, plane->pos.x * 2, buf);
}

void draw_airport(struct endpoint *airport, char *buf)
{
	attrset(COLOR_PAIR(CP_AIRPORT));
	if (snprintf(buf, 3, CHAR_AIRPORT "%c", airport->num) < 0) {
		perror("draw_airport: snprintf");
	}
	mvaddstr(airport->pos.y, airport->pos.x * 2, buf);
}

void draw_exit(struct endpoint *ext, char *buf)
{
	attrset(COLOR_PAIR(CP_EXIT));
	if (snprintf(buf, 2, "%c", ext->num) < 0) {
		perror("draw_exit: snprintf");
	}
	mvaddstr(ext->pos.y, ext->pos.x * 2, buf);
}

void draw_beacon(struct beacon *beacon, char *buf)
{
	if (snprintf(buf, 3, CHAR_BEACON "%c", beacon->num) < 0) {
		perror("draw_beacon: snprintf");
	}
	mvaddstr(beacon->pos.y, beacon->pos.x * 2, buf);
}

void draw_endpoint(struct endpoint *ep, char *buf)
{
	switch (ep->type) {
	case EP_AIRPORT:
		draw_airport(ep, buf);
		break;
	case EP_EXIT:
		draw_exit(ep, buf);
		break;
	}
}

void clear_prev_frame(struct state *state)
{
	attrset(COLOR_PAIR(CP_NORMAL));
	for (size_t i = 0; i < state->num_planes; i++) {
		struct plane *plane = &state->planes[i];
		mvaddstr(plane->pos.y, plane->pos.x * 2, "  ");
	}
}

void draw_state(struct state *state)
{
	char buf[] = "A0";
	attrset(COLOR_PAIR(CP_BEACON));
	for (size_t i = 0; i < state->num_beacons; i++) {
		struct beacon *beacon = &state->beacons[i];
		draw_beacon(beacon, buf);
	}

	for (size_t i = 0; i < state->num_endpoints; i++) {
		struct endpoint *ep = &state->endpoints[i];
		draw_endpoint(ep, buf);
	}

	attrset(COLOR_PAIR(CP_PLANE));
	for (size_t i = 0; i < state->num_planes; i++) {
		draw_plane(state, i, buf);
	}
	refresh();
}

void render_deinit()
{
	endwin();
}
