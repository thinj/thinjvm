/*
 * thinjvm.c
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */
#include <stdio.h>

#include "config.h"
#include "debugger.h"
#include "frame.h"
#include "heap.h"

void thinjvm_run(void* heap, size_t heapSize) {
	resetVM(heap, heapSize);

	push_frame(0, startClassIndex, startAddress, TRUE);

#ifdef USE_DEBUG
	initDebugger();
	debug();
#else
	execute();
#endif

	jvmexit(0);
}

