/*
 * operandstack.h
 *
 *  Created on: Aug 22, 2010
 *      Author: hammer
 */

#ifndef OPERANDSTACK_H_
#define OPERANDSTACK_H_

#include "types.h"
#include "config.h"

typedef union __stackableOperand {
	jint jrenameint;
	jobject jref;
	u2 u2val;
} stackableOperand;

typedef enum {
	// use obscure values => gives us some kind of 'magic' effect:
	JAVAINT = 47,
	OBJECTREF = 69,
	U2 = 111
} stackType;

typedef struct __stackable {
	stackType type;
	stackableOperand operand;
} stackable;

extern stackable stack[];




stackable* pop(void);
void push(stackableOperand op, stackType type);

void operandStackPushJavaInt(jint);
void operandStackPushJavaString(char* string);
jint operandStackPopJavaInt(void);
jobject operandStackPopObjectRef(void);

void operandStackPushU2(u2 u2);
u2 operandStackPopU2(void);

void operandStackPushVariableJavaInt(u1 varnum);
void operandStackPushVariableObjectRef(u1 varnum);

void operandStackPushObjectRef(jobject jref);

void operandStackPopVariableJavaInt(u1 varnum);
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
void stack_init(void);

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
BOOL operandStackIsObjectRefAtOffsetNull(u2 offset);

#endif /* OPERANDSTACK_H_ */
