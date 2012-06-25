#define _XOPEN_SOURCE_EXTENDED = 1  /* extended character sets */
#include <stdlib.h>
#include <ncurses.h>
#include <panel.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


void start_ncurses(void)
{
        setlocale(LC_ALL,"");  // UTF-8
        initscr();             // Start ncurses
        start_color();         // Initialize color display 
        cbreak();	       // Disable line buffering
        noecho();              // Do not echo input
        curs_set(0);           // hide cursor
        nodelay(stdscr, true); // getch() is non-blocking
}

void stop_ncurses(void)
{
        endwin();
}
