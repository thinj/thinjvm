/*
 * config.h
 *
 *  Created on: Jun 23, 2011
 *      Author: hammer
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "thinjvm.h"


//#define __DEBUG(x ...)
//#define __DEBUG consout("%s(%s:%d): ", __FUNCTION__, __FILE__, __LINE__), consout



//insert heap and stack size in this file

//#define HEAP_SIZE 9000
//#define STACK_SIZE 2000


//#define USE_DEBUG

#ifdef USE_DEBUG
#define DEB(X) X
#define jvmexit(X) exit(X)
#else
#include <stdio.h>
#include "console.h"
//#include "frame.h"
#define DEB(X)

#endif

// For dumpStackTrace prototype:
#include "frame.h"

#define jvmexit(X) do {if (X) {dumpStackTrace();}  consout("jvmexit %s %d\n", __FILE__, __LINE__); thinjvm_exit(X);} while (0)



#endif /* CONFIG_H_ */
