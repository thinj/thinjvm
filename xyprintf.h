/*
 * xyprintf.h
 *
 *  Created on: Oct 30, 2010
 *      Author: hammer
 */

#ifndef XYPRINTF_H_
#define XYPRINTF_H_

typedef struct __windowDef {
	int x;
	int y;
	int width;
	int height;
} windowDef;

void clearScreen(void);

#define BOLD 1
#define INV 2
#define UNDER 4

/**
 * This method prints a formatted string
 * \param x The column to print at
 * \param y The row to print at
 * \param fontFlags The font
 * \param format The usual printf - format string
 */
void xyprintf(int x, int y, int fontFlags, char * format, ...);

/**
 * This method positions the cursor at the position (x, y) relative to
 * the 'window'
 */
void cursorTo(windowDef window, int x, int y);

/**
 * This method sets the upper left corner for the subsequent xyprinting and the max length of the
 * printed line. Printed lines shorter than 'width' are extended using white space (in the current line
 * font)
 * Note that a border is painted just outside the window; the caller should take this into consideration
 * when allocating the window sizes.
 * \param window The current window dimension
 */
void xySetWindow(windowDef window);

/**
 * This method blocks until a key is pressed on the keyboard
 * \return The value of the pressed key
 */
int getch(void);

/**
 * This method initialize the printing stuff
 */
void xyinit(void);

/**
 * This method flushes all printing to the console.
 */
void xyflush(void);

#endif /* XYPRINTF_H_ */
