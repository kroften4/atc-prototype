#ifndef __ATC_NCURSES_RENDER_H__
#define __ATC_NCURSES_RENDER_H__

#include "atc/state.h"

void render_init();

void draw_state(struct state *state);

void render_deinit();

void clear_prev_frame(struct state *state);

#endif // __ATC_NCURSES_RENDER_H__
