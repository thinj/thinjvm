/*
 * main.c
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */
#include <stdio.h>

#include "config.h"
#include "debugger.h"
#include "frame.h"
#include "heap.h"
#include "hardware.h"

int main(int argc, char **argv) {
	init_hardware();
	resetVM();

	push_frame(0, startClassIndex, startAddress, TRUE);

#ifdef USE_DEBUG
	initDebugger();
	debug();
#else
	execute();
#endif

	jvmexit(0);
	return 0;
}

