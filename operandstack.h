/*
 * operandstack.h
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#ifndef OPERANDSTACK_H_
#define OPERANDSTACK_H_

#include <stdlib.h>
#include "types.h"
#include "config.h"

//typedef union __stackableOperand {
//	jint jrenameint;
//	jobject jref;
//	u2 u2val;
//} stackableOperand;
//
//typedef enum {
//	// use obscure values => gives us some kind of 'magic' effect:
//	JAVAINT = 47,
//	OBJECTREF = 69,
//	U2 = 111
//} stackType;

//typedef struct __stackable {
//	stackType type;
//	stackableOperand operand;
//} stackable;

extern stackable* stack;

/**
 * This is the size of the stack counted in stackable entries, not in bytes:
 */
extern size_t STACK_SIZE;

/**
 * \param type
 * \returns The type as a human readable string
 */
const char* StackTypeToString(stackType type);

void pop(stackable* op);

#define push(OP, TYPE) \
do { \
	if (context.stackPointer >= STACK_SIZE) { \
		consout("stack overflow: %d", (int) (context.stackPointer)); \
		jvmexit(1); \
	} \
	stack[context.stackPointer].operand = OP; \
	stack[context.stackPointer++].type = TYPE; \
} while (0)

//void push(stackableOperand op, stackType type);

void operandStackPushJavaInt(jint);
void operandStackPushJavaLong(jlong j);
void operandStackPushJavaString(char* string);

jlong operandStackPopJavaLong(void);
jint operandStackPopJavaInt(void);
jobject operandStackPopObjectRef(void);

void operandStackPushU2(u2 u2);
u2 operandStackPopU2(void);

void operandStackPushVariableJavaInt(u1 varnum);
void operandStackPushVariableJavaLong(u1 varnum);
void operandStackPopVariableJavaLong(u1 varnum);
void operandStackPushVariableObjectRef(u1 varnum);

void operandStackPushObjectRef(jobject jref);

void operandStackPopVariableJavaInt(u1 varnum);
void operandStackPopVariableJavaLong(u1 varnum);
void operandStackPopVariableObjectRef(u1 varnum);

/**
 * This method increments the local variable identified by 'varnum' with
 * the signed value given by 'delta'
 * \param varnum The local variable index
 * \param delta The signed amount to increase with
 */
void operandStackIncrementVariableJavaInt(u1 varnum, jint delta);

/**
 * This function looks up a stackable relative to the stack pointer
 * \param offset The offset to be added to the stack pointer
 * \param st The address wherein the result shall go
 */
void getOperandRelativeToStackPointer(s1 offset, stackable* st);

/**
 * This method marks all objects on the current stack
 */
void markStackObjects(u1 mark);

/**
 * This method initializes the stack by writing integer values at all locations.
 */
void osStackInit(void);

/**
 * This method returns true if and only if the jobject variable # varnum is null
 * \param varnum The number of the variable at the stack. E.g. 'this' is typically #0
 * \return true if and only if the jobject variable # varnum is null
 */
BOOL operandStackIsVariableObjectRefNull(u1 varnum);

/**
 * This method returns true if and only if the jobject variable at address Sp - offset is null
 * \param offset The offset of the variable at the stack. The offset is subtracted from SP
 * \return true if and only if the jobject is null
 */
BOOL osIsObjectRefAtOffsetNull(u2 offset);

/**
 * This method returns a pointer to the first element at the stack
 * \return A pointer to the first element at the stack
 */
stackable* getStack();

/**
 * This function allocates a stack on heap and return the allocated stack.
 * \return The allocated stack (as a byte[])
 */
jbyteArray osAllocateStack(void);

/**
 * This function returns the current stack. Is only guaranteed to be valid during Thread#clinit().
 * \return A byte[] with the currentstack
 */
jbyteArray osGetCurrentStack();

/**
 * This function sets the current stack
 * \param The new jstack
 */
void osSetStack(jbyteArray jstack);

/**
 * This method unprotects the current stack. Shall only be used to fix a problem with the first stack:
 * The first stack is allocated on the heap, but not referenced from any objects / classes. In order of
 * avoiding GC og this stack, it is initially protected. However, when the java.lang.Thread.<clinit> has
 * been run, this initial stack is begin referenced from Thread.currentThread => it shall no longer be
 * protected. This serves this function.
 */
void osUnprotectStack();

#endif /* OPERANDSTACK_H_ */
