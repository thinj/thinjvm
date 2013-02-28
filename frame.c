/*
 * frame.c
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "heaplist.h"
#include "heap.h"
#include "frame.h"
#include "console.h"
#include "constantpool.h"
#include "instructions.h"
#include "debugger.h"
#include "vmids.h"

// The JVM 'cpu' registers etc:
contextDef context;

/**
 *  Flag for enabling scheduling.
 *  Shall be set true when java.lang.Thread has changed the way stacks and context shall be accessed.
 */
static BOOL aSchedulingEnabled = FALSE;


/*
 * Frame layout and registers:
 * sp->
 * cp-> (same address as sp, or in other words sp == cp)
 *      flags (returnFromVM)
 *      prev cp
 *      prev ci
 *      retu addr
 *      prev fp
 *      local var 3
 *      local var 2
 *      local var 1
 *      local var 0
 *      arg
 *      arg
 * fp-> arg
 */
void pop_frame() {
	// set stack pointer to fixed point at stack:
	context.stackPointer = context.contextPointer;

	// pop context including return address:
	context.flags = operandStackPopU2();
	context.contextPointer = operandStackPopU2();
	context.classIndex = operandStackPopU2();
	context.programCounter = operandStackPopU2(); // This is the return
	u2 framePointer = operandStackPopU2();
	context.stackPointer = context.framePointer;
	context.framePointer = framePointer;
	// Don't touch context.exceptionThrown
}

void push_frame(u1 localVariableCount, u2 dstClassIndex, codeIndex dest, BOOL returnFromVM) {
	// Push previous frame pointer:
	operandStackPushU2(context.framePointer);
	// Push return address:
	operandStackPushU2(context.programCounter);
	// Push current class index:
	operandStackPushU2(context.classIndex);
	// Push current context pointer:
	operandStackPushU2(context.contextPointer);
	// Push flags:
	if (returnFromVM) {
		context.flags |= RETURN_FROM_VM;
	} else {
		context.flags &= ~RETURN_FROM_VM;
	}
	operandStackPushU2(context.flags);
	// Clear return - flag:
	context.flags = 0;
	// Save address of context:
	context.contextPointer = context.stackPointer;

	size_t numberOfPushedRegisters = 5;

	context.framePointer = context.stackPointer - localVariableCount - numberOfPushedRegisters;
	context.classIndex = dstClassIndex;
	context.programCounter = dest;

}

u1 getU1FromCode() {
	u1 bytecode = code[context.programCounter];
	context.programCounter++;

	return bytecode;
}

u2 getU2FromCode() {
	int msb = getU1FromCode() & 0xff;
	int lsb = getU1FromCode() & 0xff;

	return (u2) msb * 256 + (u2) lsb;
}

s1 getS1FromCode() {
	return (s1) getU1FromCode();
}

s2 getS2FromCode(void) {
	int msb = getS1FromCode() & 0xff;
	int lsb = getS1FromCode() & 0xff;

	return (s2) msb * 256 + (s2) lsb;
}

void dumpStackTrace() {
	contextDef savedContext = context;
	BOOL addComma = FALSE;
	consout("context: PC=0x%04x  CID=0x%04x  SP=0x%04x\n", context.programCounter,
			context.classIndex, context.stackPointer);
	consout("Internal trace:\n");
	//	while (context.framePointer >= 0) {
	while (1) {
		if (addComma) {
			consout(",");
		} else {
			consout("  at: ");
			addComma = TRUE;
		}
		consout("%d", context.programCounter - 1);
		if (context.framePointer == 0) {
			break;
		}
		pop_frame();
	}
	context = savedContext;
	consout("\n");
}

#ifdef USE_DEBUG
#define MAX_BREAKPOINTS 10
typedef struct __breakpoint {
	// Is the breakpoint in use:
	BOOL used;

	// The address where the breakpoint resides:
	codeIndex addr;
}breakpoint;

static breakpoint breakpoints[MAX_BREAKPOINTS];

/**
 * This method adds or removes the breakpoint defined by the parameter 'addr'
 * \param addr The address where the breakpoint shall be added or removed
 */
void toggleBreakpoint(codeIndex addr) {
	int i;
	int freeIndex = MAX_BREAKPOINTS;
	BOOL found = FALSE;
	for (i = 0; i < MAX_BREAKPOINTS && !found; i++) {
		if (breakpoints[i].used) {
			if (addr == breakpoints[i].addr) {
				// Remove breakpoint:
				breakpoints[i].used = FALSE;
				found = TRUE;
				consout("removed breakpoint: %04x\n", addr);
			}
		} else {
			// Prepare for adding a breakpoint:
			freeIndex = i;
		}
	}
	if (!found) {
		// Then try to add the breakpoint:
		if (freeIndex < MAX_BREAKPOINTS) {
			breakpoints[freeIndex].addr = addr;
			breakpoints[freeIndex].used = TRUE;
			consout("added breakpoint: %04x\n", addr);
		} else {
			consout("Too many breakpoints: %d\n", (int) addr);
		}
	}
}

BOOL isBreakpoint(codeIndex addr) {
	int i;
	BOOL found = FALSE;
	for (i = 0; i < MAX_BREAKPOINTS && !found; i++) {
		if (breakpoints[i].used) {
			found = (addr == breakpoints[i].addr) ? TRUE : FALSE;
		}
	}

	return found;
}
#endif // USE_DEBUG
void resetVM(align_t* heap, size_t heapSize, size_t stackSize) {
	// STACK_SIZE is in count of stackables, not bytes:
	STACK_SIZE = stackSize / sizeof(stackable);

#ifdef USE_DEBUG
	int i;
	for (i = 0; i < MAX_BREAKPOINTS; i++) {
		breakpoints[i].used = FALSE;
	}
#endif // USE_DEBUG
	// Initialize heap:
	heapInit(heap, heapSize);

	// Clear static area:
	memset(&staticMemory[0], staticMemorySize, sizeof(stackable));

	// Setup initial context:
	clearContext(&context, startClassIndex, startAddress);

	// initialize stack:
	osStackInit();

	// Load all java.lang.Class instances:
	generateJavaLangClassInstances();
}

void clearContext(contextDef* context, u2 classId, codeIndex startAddress) {
	context->programCounter = startAddress;
	context->classIndex = classId;
	context->stackPointer = 0;
	context->framePointer = 0;
	context->contextPointer = 0;
	context->flags = 0;
}

#ifdef USE_DEBUG
void _validateStackables(stackable* memory, size_t length);
void singleStep(void) {
	SINGLE_STEP();
	//	u2 pcWas = context.programCounter;
	//	u1 opcode = getU1FromCode();
	//	// Lookup instruction:
	//	if (opcode >= numberOfDefinedInstructions) {
	//		consout("%04x: %02x   Unsupported bytecode!\n", pcWas, opcode);
	//		consout("SP=%04x PC=%04x FP=%04x\n", context.stackPointer, context.programCounter,
	//				context.framePointer);
	//		dumpStackTrace();
	//		jvmexit(1);
	//	}
	//	//	consout("PC=%04x: %02x", pcWas, opcode);
	//	//	consout(" SP=%04x PC=%04x FP=%04x\n", context.operandStackPointer, context.programCounter,
	//	//			framePointer);
	//
	//	// Enable this code if the VM is behaving suspicious:
	//	//_validateStackables(stack, context.operandStackPointer);
	//
	//	allIns[opcode]();
	//allInstructions[opcode].ins();
}
#endif


/**
 * \return true if scheduling is enabled
 */
BOOL frIsSchedulingEnabled() {
	return aSchedulingEnabled;
}

/**
 * This method enables scheduling.
 */
void frStartScheduling() {
	aSchedulingEnabled = TRUE;
}

// TODO Make time-base!
static int yieldCount = 0;
void tryYield() {
	if (aSchedulingEnabled) {
		if (yieldCount++ >= 100) {
			yieldCount = 0;
			jclass cls = getJavaLangClass(CLASS_ID_java_lang_Thread);
			Java_java_lang_Thread_yield(NULL, cls);
		}
	}
}
