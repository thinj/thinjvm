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

#include "jarray.h"
#include "operandstack.h"
#include "frame.h"
#include "heap.h"
#include "heaplist.h"
#include "jni.h"
#include "vmids.h"

/**
 * The size of a thread stack in count of stackables:
 */
size_t STACK_SIZE = 100;

// The current stack :
stackable* stack;

// The current stack as a java-object:
static jbyteArray aCurrentStack;

/**
 * This function returns the size of a stack in number of bytes
 */
static size_t sGetStackSizeInBytes() {
	return STACK_SIZE * sizeof(stackable);
}

jbyteArray osAllocateStack(void) {
	jbyteArray stackObject = NewByteArray(sGetStackSizeInBytes());

	return stackObject;
}

void osStackInit(void) {
	if (sGetStackSizeInBytes() >= hlGetHeapSizeInBytes()) {
		consoutli("Stack is larger than heap\n");
		jvmexit(1);
	}
	// Allocate a java object (byte[]) containing the stack:
	jbyteArray jstack = osAllocateStack();

	if (jstack == NULL) {
		consoutli("Premature out of memory; can't alloc a stack\n");
		jvmexit(1);
	}

	// Avoid garbage collection of our one and only stack (at this point).
	// The stack shall be unprotected when the stack has been referenced from the
	// first thread, see osUnprotectStack():
	heapProtect(jstack, TRUE);

	// Set as current stack:
	osSetStack(jstack);

	u2 sp = context.stackPointer;
	context.stackPointer = 0;
	while (context.stackPointer < STACK_SIZE) {
		operandStackPushJavaInt(0);
	}
	context.stackPointer = sp;
}

void osSetStack(jbyteArray jstack) {
	// Avoid dereferencing the java type each time the stack is going to be referenced:
	stack = jaGetArrayPayLoad(jstack);
	aCurrentStack = jstack;
}

void osUnprotectStack() {
	heapProtect(aCurrentStack, FALSE);
}

stackable* getStack() {
	return stack;
}

//void push(stackableOperand op, stackType type) {
//	if (context.stackPointer >= STACK_SIZE) {
//		consout("stack overflow: %d", (int) (context.stackPointer));
//		jvmexit(1);
//	}
//	stack[context.stackPointer].operand = op;
//	stack[context.stackPointer++].type = type;
////	stackable* sop = stack + (context.stackPointer++);
////	sop->operand = op;
////	sop->type = type;
//}

void pop(stackable* ret) {
	if (context.stackPointer > 0) {
		--context.stackPointer;
		*ret = *(stack + context.stackPointer);
	} else {
		consout("Operand Stack underrun: %04x\n", context.stackPointer);
		jvmexit(1);
	}
}

#define POP_VERIFY(VALUE, TYPE, FIELD) \
	do { \
		if (context.stackPointer > 0) { \
			--context.stackPointer; \
			VALIDATE_TYPE((stack + context.stackPointer)->type, TYPE); \
			VALUE = (stack + context.stackPointer)->operand.FIELD; \
		} else { \
			consout("Operand Stack underrun: %04x\n", context.stackPointer); \
			jvmexit(1); \
		} \
	} \
    while (0)

jint operandStackPopJavaInt(void) {
	jint value;

	POP_VERIFY(value, JAVAINT, jrenameint);

	return value;
}

void operandStackPushJavaLong(jlong j) {
	stackableOperand op;
	op.jrenameint = (jint) (j & 0xffffffff);
	push(op, JAVAINT);
	op.jrenameint = (jint) ((j >> 32) & 0xffffffff);
	push(op, JAVAINT);
}

jlong operandStackPopJavaLong(void) {
	jlong j;
	jint value;

	POP_VERIFY(value, JAVAINT, jrenameint);
	j = value;
	j <<= 32;
	//	j = ((jlong) value) << 32;
	POP_VERIFY(value, JAVAINT, jrenameint);
	j = j | (value & 0xffffffff);

	return j;
}

jobject operandStackPopObjectRef(void) {
	jobject value;

	POP_VERIFY(value, OBJECTREF, jref);

	return value;
}

u2 operandStackPopU2() {
	u2 value;

	POP_VERIFY(value, U2, u2val);

	return value;
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

void operandStackPushVariableJavaInt(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];

	VALIDATE_TYPE(op.type, JAVAINT);

	push(op.operand, JAVAINT);
}

void operandStackPushVariableJavaLong(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];

	VALIDATE_TYPE(op.type, JAVAINT);

	push(op.operand, JAVAINT);

	op = stack[varnum + 1 + context.framePointer];

	VALIDATE_TYPE(op.type, JAVAINT);

	push(op.operand, JAVAINT);
}

void operandStackPushVariableObjectRef(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];
	VALIDATE_TYPE(op.type, OBJECTREF);

	push(op.operand, OBJECTREF);
}

BOOL operandStackIsVariableObjectRefNull(u1 varnum) {
	stackable op = stack[varnum + context.framePointer];

	VALIDATE_TYPE(op.type, OBJECTREF);

	return op.operand.jref == NULL ? TRUE : FALSE;
}

BOOL osIsObjectRefAtOffsetNull(u2 offset) {
	stackable op = stack[context.stackPointer - offset];

	VALIDATE_TYPE(op.type, OBJECTREF);

	return op.operand.jref == NULL ? TRUE : FALSE;
}

void getOperandRelativeToStackPointer(s1 offset, stackable* st) {
	*st = stack[context.stackPointer + offset];
}

void operandStackIncrementVariableJavaInt(u1 varnum, jint delta) {
	stackType type = stack[varnum + context.framePointer].type;
	VALIDATE_TYPE(type, JAVAINT);

	stack[varnum + context.framePointer].operand.jrenameint += delta;
}

void operandStackPopVariableJavaInt(u1 varnum) {
	jint jrenameint = operandStackPopJavaInt();
	stackable* dst = &stack[varnum + context.framePointer];
	dst->operand.jrenameint = jrenameint;
	dst->type = JAVAINT;
}

void operandStackPopVariableJavaLong(u1 varnum) {
	jint ms = operandStackPopJavaInt();
	jint ls = operandStackPopJavaInt();
	stackable* dst = &stack[varnum + context.framePointer];
	dst->operand.jrenameint = ls;
	dst->type = JAVAINT;
	dst++;
	dst->operand.jrenameint = ms;
	dst->type = JAVAINT;
}

void operandStackPopVariableObjectRef(u1 varnum) {
	jobject jref = operandStackPopObjectRef();
	stack[varnum + context.framePointer].operand.jref = jref;
	stack[varnum + context.framePointer].type = OBJECTREF;
}

const char* StackTypeToString(stackType type) {
	switch (type) {
	case JAVAINT:
		return "JAVAINT";
	case OBJECTREF:
		return "OBJECTREF";
	case U2:
		return "U2";
	default:
		return "(unknown)";
	}
}

jbyteArray osGetCurrentStack() {
	return aCurrentStack;
}

