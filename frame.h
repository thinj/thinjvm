/*
 * frame.h
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#ifndef FRAME_H_
#define FRAME_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include "types.h"
#include "operandstack.h"
#include "constantpool.h"

/**
 * Flags for context:
 */
/**
 * When the RETURN_FROM_VM flag is set (happens during pop_frame, if RETURN_FROM_VM_PENDING is set)
 * the execute() - function shall return
 */
#define RETURN_FROM_VM 0x01
/**
 * The RETURN_FROM_VM_PENDING shall be set when calling the execute() - function.
 * When a pop_frame is executed, the RETURN_FROM_VM_PENDING shall be copied to the RETURN_FROM_VM
 * flag.
 */
#define RETURN_FROM_VM_PENDING 2

// The 'cpu' registers in the JVM:
typedef struct __contextDef {
	codeIndex programCounter;
	u2 stackPointer;
	u2 framePointer;
	u2 classIndex;
	// The position at the stack from where the previous context can be pop'ed:
	u2 contextPointer;
	u2 flags;
} contextDef;

extern contextDef context;


/**
 * This method pops the current frame off the stack
 */
void pop_frame();

/**
 * This method pushes a frame on stack (more or less same as an invoke<address>)
 *
 * \param localVariableCount The number of local variables including method arguments
 * \param dstClassIndex The class id of the class containing the method to call
 * \param dest The address of the method to call
 * \param returnFromVM if TRUE: When a frame is popped (using pop_frame) and this is true,
 * the execute() - function will return; if FALSE: No return will happen from the execute()
 * function
 */
void push_frame(u1 localVariableCount, u2 dstClassIndex, codeIndex dest, BOOL returnFromVM);

u1 getU1FromCode(void);
u2 getU2FromCode(void);
s1 getS1FromCode(void);
s2 getS2FromCode(void);

/**
 * This method resumes program execution program as defined in the auto generated .c - file.
 * The execution will stop when a breakpoint is met or the 'HALT' - instruction is met.
 */
void execute(void);

/**
 * This method single step the program
 */
void singleStep(void);

/**
 * This method will reset the VM. Shall be called once upon program start.
 */
void resetVM(void);

/*
 * This method dumps a stack trace of current (thread)..
 */
void dumpStackTrace();

/**
 * This method adds or removes the breakpoint defined by the parameter 'addr'
 * \param addr The address where the breakpoint shall be added or removed
 */
void toggleBreakpoint(codeIndex addr);

/**
 * This method returns true if 'addr' is a breakpoint address
 * \param addr The address to test
 */
BOOL isBreakpoint(codeIndex addr);

#ifdef  __cplusplus
}
#endif


#endif /* FRAME_H_ */
