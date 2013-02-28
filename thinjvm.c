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

void thinjvm_run(align_t* heap, size_t heapSize, size_t stackSize) {
	resetVM(heap, heapSize, stackSize);

	push_frame(0, startClassIndex, startAddress, TRUE);

#ifdef USE_DEBUG
	initDebugger();
	debug();
#else
	execute();
#endif

	jvmexit(0);
}

