/*
 * xyprintf.c
 *
 *  Created on: Oct 30, 2010
 *      Author: hammer
 */

#include "config.h"

#ifdef USE_DEBUG

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>

#include "types.h"
#include "xyprintf.h"

#define esc 27

#define cls printf("%c[2J",esc)

#define POS(row,col) printf("%c[%d;%dH",esc,row,col)

void clearScreen(void) {
	cls;
}

struct termios orig_termios;

static void exitHandler() {
	// Reset the terminal
	tcsetattr(0, TCSANOW, &orig_termios);
	xyprintf(1, 20, 0, "");
	fflush(stdout);
	printf("\n");
}

static void set_conio_terminal_mode() {
	struct termios new_termios;

	/* take two copies - one for now, one for later */
	tcgetattr(0, &orig_termios);
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	/* register cleanup handler, and set the new terminal mode */
	atexit(exitHandler);
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}

static int kbhit() {
	struct timeval tv = {0L, 50000L};
	fd_set fds;
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv);
}

static int kbread() {
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	} else {
		return c;
	}
}

int getch(void) {
	while (!kbhit()) {
		/* do some work */
	}
	int c = kbread(); /* consume the character */

	if (c == 3) {
		jvmexit(0);
	}
	return c;
}

void xyinit(void) {
	set_conio_terminal_mode();
	clearScreen(); /* clear the screen */
}

static int xOffset = 0;
static int yOffset = 0;
static int xyWidth = 0;

void xySetWindow(windowDef window) {
	xOffset = 0;
	yOffset = 0;
	xyWidth = 0;
	int x, y;
	for (x = -1; x < window.width + 1; x++) {
		xyprintf(x + window.x, window.y - 1, 0, "+");
		xyprintf(x + window.x, window.y + window.height, 0, "+");
	}

	for (y = -1; y < window.height + 1; y++) {
		xyprintf(window.x - 1, y + window.y, 0, "!");
		xyprintf(window.x + window.width, y + window.y, 0, "!");
	}

	xOffset = window.x;
	yOffset = window.y;
	xyWidth = window.width;
}

void cursorTo(windowDef window, int x, int y) {
	POS(window.y + y, window.x + x);
}

void xyprintf(int x, int y, int fontFlags, char * format, ...) {
	POS(y + yOffset, x + xOffset);
	char buffer[256];
	va_list args;
	va_start (args, format);
	vsprintf(buffer, format, args);
	if (xyWidth != 0) {
		int len = strlen(buffer);
		if (len < xyWidth) {
			memset(buffer + len, ' ', xyWidth - len);
		}
		buffer[xyWidth] = '\0';
	}
	if (fontFlags != 0) {
		char render[20];
		sprintf(render, "%c[", esc);
		if ((fontFlags & BOLD) == BOLD) {
			strcpy(render + strlen(render), "1");
			fontFlags -= BOLD;
		}
		if (fontFlags != 0) {
			strcpy(render + strlen(render), ";");
		}
		if ((fontFlags & INV) == INV) {
			strcpy(render + strlen(render), "7");
			fontFlags -= INV;
		}
		if (fontFlags != 0) {
			strcpy(render + strlen(render), ";");
		}
		if ((fontFlags & UNDER) == UNDER) {
			strcpy(render + strlen(render), "4");
			fontFlags -= UNDER;
		}
		sprintf(render + strlen(render), "m");
		printf("%s", render);
	} else {
		printf("%c[0m", esc);
	}

	printf("%s", buffer);
	va_end (args);
}

void xyflush(void) {
	fflush(stdout);
}
#endif //USE_DEBUG
