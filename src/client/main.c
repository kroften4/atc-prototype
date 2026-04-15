#include <locale.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include "vec.h"
#include "atc/types.h"

#define CP_PLANE 1
#define CP_AIRPORT 2

static volatile bool keep_running = true;

void sigint_handler(int)
{
	keep_running = false;
}

int main()
{
	(void)signal(SIGINT, sigint_handler);

	if (!setlocale(LC_ALL, "")) {
		perror("setlocale");
		return EXIT_FAILURE;
	}

	initscr();
	cbreak();
	noecho();

	if (has_colors())
		start_color();

	init_pair(CP_PLANE, COLOR_GREEN, COLOR_BLACK);
	init_pair(CP_AIRPORT, COLOR_YELLOW, COLOR_RED);

	curs_set(0);

	attrset(COLOR_PAIR(CP_PLANE));
	mvaddstr(3, 3, "A7");
	attrset(COLOR_PAIR(CP_AIRPORT));
	mvaddstr(10, 5, "^0");
	refresh();

	while (keep_running) {
	}

	endwin();
	return EXIT_SUCCESS;
}
