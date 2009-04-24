#include <stdlib.h>
#include <ncurses.h>
#include <glib.h>

/* Synopsis: lovexmpp <mount point> <jid> */

int parseinput(int ch) {
	int x, y, mx, my;

	switch(ch) {
		case KEY_F(10):
			return 0;
		case '\n':
	//		break;
		default:
			getyx(stdscr, y, x);
			getmaxyx(stdscr, my, mx);
			mvprintw(my - 1, x, "%c", ch);
	}
	refresh();
	return 1;
}

int main(int argc, char** argv) {
	initscr();
	noecho();
	keypad(stdscr, TRUE);

	while(parseinput(getch()));

	endwin();
	return 0;
}
