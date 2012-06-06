/*
 * operandstack.c
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "console.h"

#include "operandstack.h"
#include "frame.h"
#include "heap.h"

stackable stack[STACK_SIZE];

void stack_init(void) {
	u2 sp = context.stackPointer;
	context.stackPointer = 0;
	while (context.stackPointer < STACK_SIZE) {
		operandStackPushJavaInt(0);
	}
	context.stackPointer = sp;
}


void push(stackableOperand op, stackType type) {
	if (context.stackPointer >= STACK_SIZE) {
		consout("stack overflow: %d", (int) (context.stackPointer));
		jvmexit(1);
	}
	stack[context.stackPointer].operand = op;
	stack[context.stackPointer++].type = type;
}

stackable* pop(void) {
	stackable* ret;
	if (context.stackPointer > 0) {
		--context.stackPointer;
		ret = stack + context.stackPointer;
	} else {
		consout("Operand Stack under run: %04x\n", context.stackPointer);
		jvmexit(1);
	}

	return ret;
}

void operandStackPushObjectRef(jobject jref) {
	stackableOperand op;
	op.jref = jref;
	push(op, OBJECTREF);
}

void operandStackPushJavaInt(jint jrenameint) {
	stackableOperand op;
	op.jrenameint = jrenameint;
	push(op, JAVAINT);
}


void operandStackPushU2(u2 u2) {
	stackableOperand op;
	op.u2val = u2;
	push(op, U2);
}

u2 operandStackPopU2() {
	stackable* op = pop();
	if (op->type != U2) {
		consout("Type mismatch; expected U2, got %d\n", op->type);
		jvmexit(1);
	}

	return op->operand.u2val;
}

void operandStackPushVariableJavaInt(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];
	if (op.type != JAVAINT) {
		consout("Type mismatch; expected JAVAINT, got %d\n", op.type);
		jvmexit(1);
	}

	push(op.operand, JAVAINT);
}

void operandStackPushVariableObjectRef(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];
	if (op.type != OBJECTREF) {
		consout("1 Type mismatch; expected OBJECTREF, got %d\n", op.type);
		jvmexit(1);
	}

	push(op.operand, OBJECTREF);
}

BOOL operandStackIsVariableObjectRefNull(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];
	if (op.type != OBJECTREF) {
		consout("1 Type mismatch; expected OBJECTREF, got %d\n", op.type);
		jvmexit(1);
	}

	return op.operand.jref == NULL ? TRUE : FALSE;
}

BOOL operandStackIsObjectRefAtOffsetNull(u2 offset) {
	stackable op = stack[context.stackPointer - offset];
	if (op.type != OBJECTREF) {
		consout("1 Type mismatch; expected OBJECTREF, got %d\n", op.type);
		jvmexit(1);
	}

	return op.operand.jref == NULL ? TRUE : FALSE;
}


void getOperandRelativeToStackPointer(s1 offset, stackable* st) {
	*st = stack[context.stackPointer + offset];
}

void operandStackIncrementVariableJavaInt(u1 varnum, jint delta) {
	stackType type = stack[varnum + context.framePointer].type;
	if (type != JAVAINT) {
		consout("Expected JAVAINT; got %d", type);
		jvmexit(1);
	}

	stack[varnum + context.framePointer].operand.jrenameint += delta;
}

void operandStackPopVariableJavaInt(u1 varnum) {
	jint jrenameint = operandStackPopJavaInt();
	stackable* dst = &stack[varnum + context.framePointer];
	dst->operand.jrenameint = jrenameint;
	dst->type = JAVAINT;
}

void operandStackPopVariableObjectRef(u1 varnum) {
	jobject jref = operandStackPopObjectRef();
	stack[varnum + context.framePointer].operand.jref = jref;
	stack[varnum + context.framePointer].type = OBJECTREF;
}

jint operandStackPopJavaInt(void) {
	stackable* op = pop();
	if (op->type != JAVAINT) {
		consout("Type mismatch; expected JAVAINT, got %d\n", op->type);
		jvmexit(1);
	}

	return op->operand.jrenameint;
}

jobject operandStackPopObjectRef(void) {
	stackable* op = pop();
	if (op->type != OBJECTREF) {
		consout("2 Type mismatch; expected OBJECTREF, got %d", op->type);
		jvmexit(1);
	}

	return op->operand.jref;
}

