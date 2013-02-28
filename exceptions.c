/*
 * exceptions.c
 *
 *  Created on: Sep 23, 2012
 *      Author: hammer
 */

#include "constantpool.h"
#include "frame.h"
#include "heap.h"
#include "jni.h"
#include "objectaccess.h"
#include "vmids.h"
#include "heaplist.h"

/**
 * This method looks up the exception handler for the exception. Note that program counter is used when
 * looking up the handler.
 *
 * \param containingClassId The class containing the code throwing the exception
 * \param exceptionClassId The exception thrown
 *
 * \return The found handler or NULL, if no handler for the exception.
 */
const exceptionHandler* getExceptionHandler(u2 containingClassId, u2 exceptionClassId) {
	int i;
	const exceptionHandler* hit = NULL;
	do {
		containingClassId = context.classIndex;
		// The tables expect PC to be incremented after the instruction:
		codeIndex pc = context.programCounter - 1;
		for (i = 0; i < numberOfAllExceptionHandersInAllClasses; i++) {
			const exceptionHandler* p = NULL;
			p = &(allExceptionHandersInAllClasses[i]);

			BOOL classOk = p->exceptionClassId == exceptionClassId;
			if (!classOk) {
				classOk = is_S_SubClassing_T(exceptionClassId, p->exceptionClassId);
			}
			if (p->classId == containingClassId && classOk && p->startPC <= pc && p->endPC > pc) {
				hit = p;
				break;
			}
		}
		if (hit == NULL) {
			if (context.framePointer > 0) {
				pop_frame();
			} else {
				// No exception handler; return null:
				break;
			}
		}
	} while (hit == NULL);

	return hit;
}

/**
 * This method throws an exception
 *
 * \param exception The exception to throw
 */
void throwException(jobject exception) {
	// See VM Spec section 3.10:file:///tools/vmspec/Overview.doc.html#15494
	// Get class id of exception:
	u2 exceptionClassId = oaGetClassIdFromObject(exception);
	//__DEBUG("containing classId: %d\n", context.classIndex);
	//__DEBUG("class id of exception: %d\n", (int) exceptionClassId);

	contextDef contextCopy = context;
	const exceptionHandler* p = getExceptionHandler(context.classIndex, exceptionClassId);

	if (p != NULL) {
		//__DEBUG("Found handler: %04x\n", p->handlerPC);
		operandStackPushObjectRef(exception);
		context.programCounter = p->handlerPC;
	} else {
		HEAP_VALIDATE;
		heap_dump();
		context = contextCopy;
		consout("Uncaught exception (%d), terminating thread: \n", (int)exceptionClassId);
		jvmexit(1);
	}
	context.exceptionThrown = TRUE;
}

/**
 * This method throws a Null Pointer Exception
 */
void throwNullPointerException(void) {
	jobject npe = newObject(CLASS_ID_java_lang_NullPointerException);

	operandStackPushObjectRef(npe);

	call_instance_method(npe, LINK_ID_java_lang_NullPointerException__init____V);

	throwException(npe);
}

/**
 * This method throws a Negative Array Size Exception
 */
void throwNegativeArraySizeException() {
	jobject npe = newObject(CLASS_ID_java_lang_NegativeArraySizeException);

	operandStackPushObjectRef(npe);

	call_instance_method(npe, LINK_ID_java_lang_NegativeArraySizeException__init____V);

	throwException(npe);
}

void throwArrayIndexOutOfBoundsException(jint index, jint arrayLength) {
	jobject except = newObject(CLASS_ID_java_lang_ArrayIndexOutOfBoundsException);
	operandStackPushObjectRef(except);
	operandStackPushJavaInt(index);

	call_instance_method(except, LINK_ID_java_lang_ArrayIndexOutOfBoundsException__init___I_V);

	throwException(except);
}

void throwArithmeticException(const char* cause) {
	jobject except = newObject(CLASS_ID_java_lang_ArithmeticException);

	operandStackPushObjectRef(except);
	jobject jstr = NewString((jchar*)cause);
	operandStackPushObjectRef(jstr);

	call_instance_method(except, LINK_ID_java_lang_ArithmeticException__init___Ljava_lang_String__V);

	throwException(except);
}

void throwClassCastException(u2 classId_S, u2 classId_T) {
	jobject exception = newObject(CLASS_ID_java_lang_ClassCastException);

	operandStackPushObjectRef(exception);

	call_instance_method(exception, LINK_ID_java_lang_ClassCastException__init____V);

	throwException(exception);
}

void throwOutOfMemoryError() {
	u2 classId = CLASS_ID_java_lang_OutOfMemoryError;
	u2 memberId = LINK_ID_java_lang_OutOfMemoryError_getInstance___Ljava_lang_OutOfMemoryError_;
	call_static_method(classId, memberId);

	// Pop exception:
	jobject exception = operandStackPopObjectRef();

	throwException(exception);
}
