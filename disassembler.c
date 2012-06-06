/*
 * disassembler.c
 *
 *  Created on: Nov 5, 2010
 *      Author: hammer
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"

#ifdef USE_DEBUG

#include "disassembler.h"
#include "types.h"
#include "constantpool.h"
#include "instructions.h"

assemblyLine* disassembly = NULL;
size_t disassemblySize = 0;

void disassembleAll(void) {
	codeIndex index = 0;

	while (index < codeSize) {
		u1 opcode = code[index];
		insWithOpcode* insp = lookupInstruction(opcode);
		if (insp != NULL) {
			index += insp->length;
			disassemblySize++;
		} else {
			DEB(fflush(stdout));
			DEB(fprintf(stderr, "%04x: %02x   Unsupported bytecode!\n", index, opcode));
			DEB(fprintf(stderr, "SP=%04x PC=%04x FP=%04x\n", context.stackPointer,
					context.programCounter, context.framePointer));
			jvmexit(1);
		}
	}

	disassembly = malloc(sizeof(assemblyLine) * disassemblySize);

	index = 0;
	int row = 0;
	while (index < codeSize) {
		u1 opcode = code[index];
		insWithOpcode* insp = lookupInstruction(opcode);
		assemblyLine* line = &disassembly[row++];
		line->programCounter = index;
		sprintf(line->instr, "%04x:", index);
		int i;
		for (i = 0; i < 5; i++) {
			if (i < insp->length) {
				sprintf(line->instr + strlen(line->instr), " %02x", code[index + i]);
			} else {
				sprintf(line->instr + strlen(line->instr), "   ");
			}
		}
		index += insp->length;
		sprintf(line->instr + strlen(line->instr), "  %s", insp->name);
	}
}
#endif //USE_DEBUG
