/*
 * disassembler.h
 *
 *  Created on: Nov 5, 2010
 *      Author: hammer
 */

#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include <stdio.h>
#include "types.h"

typedef struct __assemblyLine {
	codeIndex programCounter;
	char instr[80];
} assemblyLine;


extern assemblyLine* disassembly;
extern size_t disassemblySize;

void disassembleAll(void);

#endif /* DISASSEMBLER_H_ */
