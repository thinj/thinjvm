/*
 * debugger.h
 *
 *  Created on: Nov 5, 2010
 *      Author: hammer
 */

#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include "config.h"

// The max. line length in the console window:
#define CONSOLE_LINE_LENGTH 100

#ifdef USE_DEBUG
#include "types.h"

void initDebugger(void);

/**
 * This method updates all debug 'windows' and blocks until any key is pressed
 */
void debug(void);


/**
 * This method prints a single character to the console window
 * \param ch The character to print
 */
void debugger_putchar(char ch);

/**
 * This method repaints all windows
 */
void repaint();

#endif // USE_DEBUG
#endif /* DEBUGGER_H_ */
