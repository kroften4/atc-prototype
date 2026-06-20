#include "atc-ncurses/render.h"
#include "atc-ncurses/input.h"
#include "atc/level.h"
#include "atc/plane.h"
#include "atc/state.h"
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static volatile bool keep_running = true;

void sigint_handler(int)
{
	keep_running = false;
}

void level_init_default(struct level *level)
{
	level->bounds = (struct vec){ 30, 21 };

	level->num_beacons = 2;
	level->beacons = calloc(level->num_beacons, sizeof(struct beacon));
	level->beacons[0] = (struct beacon){ .pos = { 12, 7 } };
	level->beacons[1] = (struct beacon){ .pos = { 12, 17 } };

	level->num_airports = 2;
	level->airports = calloc(level->num_airports, sizeof(struct endpoint));
	level->airports[0] = (struct endpoint){ .pos = { 20, 15 }, .dir = DIR_0 };
	level->airports[1] = (struct endpoint){ .pos = { 20, 18 }, .dir = DIR_90 };

	level->num_exits = 8;
	level->exits = calloc(level->num_exits, sizeof(struct endpoint));
	level->exits[0] = (struct endpoint){ .pos = { 12, 0 }, .dir = DIR_180 };
	level->exits[1] = (struct endpoint){ .pos = { 29, 0 }, .dir = DIR_225 };
	level->exits[2] = (struct endpoint){ .pos = { 29, 7 }, .dir = DIR_270 };
	level->exits[3] = (struct endpoint){ .pos = { 29, 17 }, .dir = DIR_270 };
	level->exits[4] = (struct endpoint){ .pos = { 9, 20 }, .dir = DIR_45 };
	level->exits[5] = (struct endpoint){ .pos = { 0, 13 }, .dir = DIR_90 };
	level->exits[6] = (struct endpoint){ .pos = { 0, 7 }, .dir = DIR_90 };
	level->exits[7] = (struct endpoint){ .pos = { 0, 0 }, .dir = DIR_135 };

	level->spawn_coeff = 10; // 10
	level->update_interval = 5; // 5
}

void level_deinit(struct level *level)
{
	free(level->airports);
	free(level->beacons);
	free(level->exits);
}

int main()
{
	(void)signal(SIGINT, sigint_handler);

	if (!setlocale(LC_ALL, "")) {
		perror("setlocale");
		return EXIT_FAILURE;
	}
	srand(time(NULL));

	struct renderer renderer = {};
	render_init(&renderer);

	struct state state = {};

	struct level level = {};
	level_init_default(&level);
	state_init(&state, &level);
	level_deinit(&level);

	struct input_data input_data = {};
	input_init(&input_data, &state, &renderer);

	struct flight_end_data fle_data = {};

	while (keep_running) {
		draw_state(&renderer, &state);
		process_input_during_update_interval(&input_data);
		if (arena_tick(&state, &fle_data)) {
			draw_state(&renderer, &state);
			break;
		}
	}

	print_game_over(&renderer, &state, fle_data);
	char ch = 0;
	while ((ch = getch()) != ' ') {
	}
	state_deinit(&state);
	render_deinit(&renderer);

	return EXIT_SUCCESS;
}
