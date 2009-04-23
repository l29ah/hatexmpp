#include <stdlib.h>
#include <ncurses.h>

/* Synopsis: lovexmpp <mount point> <jid> */

int parseinput(int ch) {
	switch(ch) {
		case KEY_F(10):
			return 1;
		default:
			printw("%c", ch);
	}
	refresh();
	return 0;
}

int main(int argc, char** argv) {
	initscr();
	noecho();
	keypad(stdscr, TRUE);

	while(parseinput(getch()));

	endwin();
	return 0;
}
