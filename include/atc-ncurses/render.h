#ifndef __ATC_NCURSES_RENDER_H__
#define __ATC_NCURSES_RENDER_H__

#include "atc/state.h"
#include <ncurses.h>

struct renderer
{
    WINDOW *win_radar;
    WINDOW *win_comms;
    WINDOW *win_status;
};

void render_init(struct renderer *rndr);

void draw_state(struct renderer *rndr, struct state *state);

void render_deinit(struct renderer *rndr);

void print_game_over(struct renderer *rndr, struct state *state,
                     struct flight_end_data fle_data);

int print_command(struct renderer *rndr, int x_offset, char *buf);
void clear_command(struct renderer *rndr, int x_offset, size_t len);
void cmd_reset_cursor(struct renderer *rndr);

void print_cmd_err(struct renderer *rndr, int underline_x_offset,
                   int underline_len, const char *err_msg_buf);

#endif // __ATC_NCURSES_RENDER_H__
