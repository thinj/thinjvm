/*
 * config.h
 *
 *  Created on: Jun 23, 2011
 *      Author: hammer
 */

#ifndef CONFIG_H_
#define CONFIG_H_


//insert heap and stack size in this file

#define HEAP_SIZE 9000
#define STACK_SIZE 1000


//#define USE_DEBUG

#ifdef USE_DEBUG
#define DEB(X) X
#define jvmexit(X) exit(X)
#define PUTCHAR(x) debugger_putchar(x);
#else
#include <stdio.h>
#include "console.h"
//#include "frame.h"
#define DEB(X)
#define jvmexit(X) do {if (X) {dumpStackTrace();}  consout("jvmexit %s %d\n", __FILE__, __LINE__); exit(X);} while (0)
#define PUTCHAR(x) putchar(x)
#endif

// #define NO_STD_LIB


#endif /* CONFIG_H_ */
