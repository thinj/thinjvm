/*
 * instructions.h
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_
#include "types.h"
#include "frame.h"

typedef void (*instruction)(void);
#define INSTRUCTION(x) void x(void)

/**
 * Table for opcode and the associated instruction
 */
typedef struct __inWithOpcode {
	instruction ins;
	u1 opcode;
	char* name;
	u1 length;
} insWithOpcode;

// Testing:
extern const instruction allIns[];


extern const insWithOpcode allInstructions[];

/**
 * This method throws an OutOfMemoryError
 */
void throwOutOfMemoryError(void);

/**
 * This method throws a Null Pointer Exception
 */
void throwNullPointerException(void);


#endif /* INSTRUCTIONS_H_ */
