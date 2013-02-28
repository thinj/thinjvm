/*
 * console.h
 *
 *  Created on: Nov 10, 2010
 *      Author: hammer
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_


/**
 * use this for all printing
 */
void consout(char * format, ...);

/**
 * This works as consout but prepend the line with source identification
 */
#define consoutli(args...) do { consout("%s:%d ", __FILE__, __LINE__); consout(args); } while (0)

/**
 * To be implemented by target platform code. Prints the supplied character on stdout
 */

void thinj_putchar(char ch);

#endif /* CONSOLE_H_ */
