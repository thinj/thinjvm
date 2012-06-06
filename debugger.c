/*
 * debugger.c
 *
 *  Created on: Nov 5, 2010
 *      Author: hammer
 */

#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"

#ifdef USE_DEBUG

#include <string.h>
#include "disassembler.h"
#include "xyprintf.h"
#include "debugger.h"
#include "console.h"
#include "frame.h"
#include "heap.h"

#define CONSOLE_LINES 10
static windowDef stackWindow = { 2, 2, 17, 20 };
static windowDef disassWindow = { 20, 2, 52, 20 };
static windowDef registerWindow = { 2, 23, 70, 1 };
static windowDef heapWindow = { 2, 25, 70, 1 };
static windowDef consoleWindow = { 2, 27, 70, CONSOLE_LINES };
static windowDef inputWindow = { 2, 38, 70, 1 };

typedef enum {
	ONE_KEY, LINE
} INPUT_STATE;

static INPUT_STATE inputState = ONE_KEY;

typedef struct __consoleLine {
	char buffer[CONSOLE_LINE_LENGTH];
	int fontFlags;
} consoleLine;

static consoleLine consoleLines[CONSOLE_LINES];
#define COMMAND_LINE_LENGTH 100
static char commandLine[COMMAND_LINE_LENGTH];

static void exitHandler(void) {
	debug();
	consout("\n");
}

void initDebugger(void) {
	disassembleAll();

	xyinit();
	int i;
	for (i = 0; i < CONSOLE_LINES; i++) {
		strcpy(consoleLines[i].buffer, "");
		consoleLines[i].fontFlags = 0;
	}
	// Flush all windows before exiting:
	atexit(exitHandler);
}

void _debugger_consout(char* s) {
	// Scroll lines:
	int i;
	for (i = 1; i < CONSOLE_LINES; i++) {
		memcpy(&consoleLines[i - 1], &consoleLines[i], sizeof(consoleLine));
	}
	strncpy(consoleLines[CONSOLE_LINES - 1].buffer, s, CONSOLE_LINE_LENGTH);
	consoleLines[CONSOLE_LINES - 1].buffer[CONSOLE_LINE_LENGTH - 1] = '\0';
	consoleLines[CONSOLE_LINES - 1].fontFlags = 0;
}

void debugger_putchar(char ch) {
	int len = strlen(consoleLines[CONSOLE_LINES - 1].buffer);
	if (ch == '\n' || len >= CONSOLE_LINE_LENGTH) {
		// Scroll lines:
		int i;
		for (i = 1; i < CONSOLE_LINES; i++) {
			memcpy(&consoleLines[i - 1], &consoleLines[i], sizeof(consoleLine));
		}
		consoleLines[CONSOLE_LINES - 1].fontFlags = 0;
		consoleLines[CONSOLE_LINES - 1].buffer[0] = '\0';
	} else {
		// Append to current line:
		consoleLines[CONSOLE_LINES - 1].buffer[len] = ch;
		consoleLines[CONSOLE_LINES - 1].buffer[len + 1] = '\0';
	}
}

//void debugger_consout(char* s) {
//	int len = strlen(s);
//	int i;
//	for (i = 0; i < len; i++) {
//		debugger_putchar(s[i]);
//	}
//}

//void debugger_consout(char* s) {
//	int len = strlen(s);
//	int ix;
//	for (ix = 0; ix < len; ix++) {
//		if (s[ix] == '\n') {
//			// Scroll lines:
//			int i;
//			for (i = 1; i < CONSOLE_LINES; i++) {
//				memcpy(&consoleLines[i - 1], &consoleLines[i], sizeof(consoleLine));
//			}
//			consoleLines[CONSOLE_LINES - 1].fontFlags = 0;
//			[CONSOLE_LINES - 1].buffer[0] = '\0';
//		}
//		else {
//			int line_len = strlen([CONSOLE_LINES - 1].buffer);
//
//			consoleLines
//		}
//		strncpy(consoleLines[CONSOLE_LINES - 1].buffer, s, CONSOLE_LINE_LENGTH);
//		consoleLines[CONSOLE_LINES - 1].buffer[CONSOLE_LINE_LENGTH - 1] = '\0';
//	}
//}

static int programCounterToDisassLine(codeIndex programCounter) {
	int programLine;
	for (programLine = 0; programLine < disassemblySize; programLine++) {
		if (disassembly[programLine].programCounter >= programCounter) {
			break;
		}
	}

	return programLine;
}

static int disassTopLine = 0;
static codeIndex disassFocusAddress = 0;

static void moveDisassFocusDown() {
	int disassLine = programCounterToDisassLine(disassFocusAddress);
	int i;
	for (i = 1; i < 10 && disassLine == programCounterToDisassLine(disassFocusAddress + i); i++) {
	}
	if (disassLine != programCounterToDisassLine(disassFocusAddress + i)) {
		disassFocusAddress += i;
	}
}

static void showDisass(void) {
	xySetWindow(disassWindow);
	int row;

	int programLine = programCounterToDisassLine(disassFocusAddress);

	if (programLine < disassTopLine) {
		// Line to focus is before the current displayed lines
		disassTopLine = programLine;
	} else if (programLine >= disassTopLine + disassWindow.height) {
		// Line to focus is after the current displayed lines
		disassTopLine = programLine - disassWindow.height + 1;
	}
	// else: Line to focus is within current displayed lines

	for (row = 0; row < disassWindow.height; row++) {
		int index = disassTopLine + row;
		if (index < disassemblySize) {
			int font = 0;
			if (disassembly[index].programCounter == context.programCounter) {
				font = INV;
			}
			if (disassembly[index].programCounter == disassFocusAddress) {
				font += BOLD;
			}
			xyprintf(0, row, font, "%c %s", isBreakpoint(disassembly[index].programCounter) ? '*'
					: ' ', disassembly[index].instr);
		} else {
			xyprintf(0, row, 0, "");
		}
	}
}

static void showRegisters(void) {
	xySetWindow(registerWindow);
	xyprintf(0, 0, 0, " SP=%04x  PC=%04x  FP=%04x  CID=%04x  CP=%04x", context.stackPointer,
			context.programCounter, context.framePointer, context.classIndex, context.contextPointer);
}

static void showHeap(void) {
	xySetWindow(heapWindow);
	heapListStat usedStat;
	heapListStat freeStat;
	gcStat gc;
	getHeapStat(&usedStat, &freeStat, &gc);

	xyprintf(0, 0, 0, " Total: %d  Free: %d/%d  Used %d/%d GCs: %d", HEAP_SIZE, freeStat.size,
			freeStat.count, usedStat.size, usedStat.count, gc.markAndSweepCount);
}

static void showInputWindow(void) {
	xySetWindow(inputWindow);
	char* format = inputState == ONE_KEY ? "%s" : ":%s";

	xyprintf(0, 0, 0, format, commandLine);
}

static int stackItemIndex = 0;
static void showStack(void) {
	// First line is where SP points to
	xySetWindow(stackWindow);
	int row;

	// Let topItemIndex = index of the top item on the stack
	int topItemIndex = context.stackPointer;// - 1 - stackWindow.height;

	if (topItemIndex < stackItemIndex) {
		// Scroll down:
		stackItemIndex = topItemIndex;
	} else if (topItemIndex >= stackItemIndex + stackWindow.height) {
		// Scroll up:
		stackItemIndex = topItemIndex - stackWindow.height + 1;
	}
	// else: No scrolling

	// Paint from buttom and up:
	for (row = 0; row < stackWindow.height; row++) {
		int reverseRow = stackWindow.height - row - 1;
		int index = row + stackItemIndex;
		stackable st = stack[index];
		char value[20];
		char type;
		switch (st.type) {
		case JAVAINT:
			type = 'I';
			sprintf(value, "%08lx", (long unsigned) st.operand.jrenameint);
			break;
		case OBJECTREF:
			type = 'A';
			// TODO detect array type ('[Z', '[I' etc)
			sprintf(value, "%08lx", (long) st.operand.jref);
			break;
			//		case ARRAYREF:
			//			type = '[';
			//			sprintf(value, "%08lx", (long) st.operand.jref);
			//			break;
		case U2:
			type = '2';
			sprintf(value, "%04x", st.operand.u2val);
			break;
		default:
			type = '?';
			break;
		}
		int font;
		if (index == context.framePointer) {
			font = INV;
		} else if (row + stackItemIndex == context.stackPointer) {
			font = INV;
		} else if (row + stackItemIndex < context.stackPointer && index
				> context.framePointer) {
			font = BOLD;
		} else {
			font = 0;
		}

		xyprintf(0, reverseRow, font, " %04x %c %08s", index, type, value);
	}
}

static void showConsole(void) {
	xySetWindow(consoleWindow);
	int i;
	for (i = 0; i < CONSOLE_LINES; i++) {
		xyprintf(0, i, consoleLines[i].fontFlags, " %s", consoleLines[i].buffer);
	}

}

/**
 * This method repaints all windows
 */
void repaint() {
	showDisass();
	showRegisters();
	showHeap();
	showStack();
	showConsole();
	showInputWindow();
	if (inputState == LINE) {
		cursorTo(inputWindow, strlen(commandLine) + 1, 0);
	}

	xyflush();
}

typedef enum {
	RUN, GC, BREAKPOINT, NUMBER, EOL, ERROR_TOKEN
} TOKEN;

static char* commandToken;
// This is set if the token is a NUMBER:
static int numberValue;

/**
 * This method extracts the next token from the command line
 */
static TOKEN nextToken() {
	TOKEN tok;

	char *token = strsep(&commandToken, " \t");

	if (token == NULL) {
		tok = EOL;
	} else if (strcmp(token, "gc") == 0) {
		tok = GC;
	} else if (strcmp(token, "run") == 0) {
		tok = RUN;
	} else if (strcmp(token, "br") == 0) {
		tok = BREAKPOINT;
	} else {
		int i;
		BOOL isDigit = TRUE;
		numberValue = 0;
		for (i = 0; i < strlen(token) && isDigit; i++) {
			isDigit = isxdigit(token[i]);
			if (isDigit) {
				numberValue = 16 * numberValue;
				if (token[i] > '9') {
					numberValue += 10 + toupper(token[i]) - 'A';
				} else {
					numberValue += token[i] - '0';
				}
			}
		}
		if (isDigit) {
			tok = NUMBER;
		} else {
			tok = ERROR_TOKEN;
		}
	}
	return tok;
}

void executeCommand() {
	// Copy command line for error message:
	char commandCopy[COMMAND_LINE_LENGTH];
	strcpy(commandCopy, commandLine);

	commandToken = commandLine;

	TOKEN tok = nextToken();
	if (tok == RUN) {
		execute();
		consout("(:run ended)\n");
	} else if (tok == GC) {
		markAndSweep();
	} else if (tok == BREAKPOINT) {
		tok = nextToken();
		if (tok == NUMBER) {
			codeIndex addr = (codeIndex) numberValue;
			toggleBreakpoint(addr);
		} else {
			consout("Error: Expected address");
		}
	} else {
		consout("Syntax error: %s", commandCopy);
	}
}

void debug(void) {
	// start with focus at the program counter:
	disassFocusAddress = context.programCounter;
	codeIndex lastProgramCounter = context.programCounter;

	while (TRUE) {
		do {
			if (lastProgramCounter != context.programCounter) {
				disassFocusAddress = context.programCounter;
				lastProgramCounter = context.programCounter;
			}
			repaint();

			// Wait for command:
			int c = getch();
			switch (inputState) {
			case ONE_KEY:
				if (c == ':') {
					inputState = LINE;
					strcpy(commandLine, "");
				} else if (c == 's') {
					singleStep();
				} else if (c == 'd') {
					moveDisassFocusDown();
				}
				break;
			case LINE:
				if (32 <= c && c < 127) {
					sprintf(commandLine, "%s%c", commandLine, c);
				} else if (c == 27) {
					inputState = ONE_KEY;
				} else if (c == 13) {
					repaint();
					executeCommand(commandLine);
					inputState = ONE_KEY;
				}
				break;
			default:
				break;
			}
		} while (inputState == LINE);
	}
}

#endif //USE_DEBUG
