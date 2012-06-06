/*
 * instruction.c
 *
 *  Created on: Aug 21, 2010
 *      Author: hammer
 */
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "console.h"
#include "frame.h"
#include "heap.h"
#include "instructions.h"
#include "debugger.h"
#include "trace.h"
#include "jni.h"

extern stackable staticMemory[];

static jobject allocJavaLangString(const char* value);

/**
 * This method allocates an instance of the specified object. No constructors are invoked.

 * \param classId Identifies the class to instantiate

 * \return The allocated object or null, if out of mem has been thrown
 */
static jobject newObject(u2 classId) {
	BEGIN;
	u2 size;
	getClassSize(classId, &size);

	CALL(jobject jref = heapAllocObject(size, classId));

	END;
	return jref;
}

//#define __DEBUG(x ...)
#define __DEBUG consout("%s(%s:%d): ", __FUNCTION__, __FILE__, __LINE__), consout

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
static void throwException(jobject exception) {
	// See VM Spec section 3.10:file:///tools/vmspec/Overview.doc.html#15494
	// Get class id of exception:
	u2 exceptionClassId = OBJECT_REF_TO_HEAP_ELEMENT(exception)->classId;
	//__DEBUG("containing classId: %d\n", context.classIndex);
	//__DEBUG("class id of exception: %d\n", (int) exceptionClassId);

	contextDef contextCopy = context;
	const exceptionHandler* p = getExceptionHandler(context.classIndex, exceptionClassId);

	if (p != NULL) {
		//__DEBUG("Found handler: %04x\n", p->handlerPC);
		operandStackPushObjectRef(exception);
		context.programCounter = p->handlerPC;
	} else {
		context = contextCopy;
		consout("Uncaught exception, terminating thread: \n");
		jvmexit(1);
	}
}

/**
 * This method throws a Null Pointer Exception
 */
void throwNullPointerException(void) {
	jobject npe = newObject(getClassBuildinDependecy(NULL_POINTER_EXCEPTION_CLASS));

	operandStackPushObjectRef(npe);

	call_instance_method(npe, getMemberBuildinDependecy(NULL_POINTER_EXCEPTION_CONSTRUCTOR));

	throwException(npe);
}

/**
 * This method throws a Negative Array Size Exception
 */
void throwNegativeArraySizeException() {
	jobject npe = newObject(getClassBuildinDependecy(NEGATIVE_ARRAY_SIZE_EXCEPTION_CLASS));

	operandStackPushObjectRef(npe);

	call_instance_method(npe, getMemberBuildinDependecy(NEGATIVE_ARRAY_SIZE_EXCEPTION_CONSTRUCTOR));

	throwException(npe);
}


void throwArrayIndexOutOfBoundsException(jint index, jint arrayLength) {
	jobject except = newObject(getClassBuildinDependecy(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS));

	operandStackPushObjectRef(except);
	operandStackPushJavaInt(index);

	call_instance_method(except,
			getMemberBuildinDependecy(ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CONSTRUCTOR));

	throwException(except);
}

void throwArithmeticException(const char* cause) {
	jobject except = newObject(getClassBuildinDependecy(ARITHMETIC_EXCEPTION_CLASS));

	operandStackPushObjectRef(except);
	jobject jstr = allocJavaLangString(cause);
	operandStackPushObjectRef(jstr);

	call_instance_method(except, getMemberBuildinDependecy(ARITHMETIC_EXCEPTION_CONSTRUCTOR));

	throwException(except);
}

void throwClassCastException(u2 classId_S, u2 classId_T) {
	jobject exception = newObject(getClassBuildinDependecy(CLASS_CAST_EXCEPTION_CLASS));

	operandStackPushObjectRef(exception);

	call_instance_method(exception, getMemberBuildinDependecy(CLASS_CAST_EXCEPTION_CONSTRUCTOR));

	throwException(exception);
}

void throwOutOfMemoryError() {
	u2 classId = getClassBuildinDependecy(OUT_OF_MEMORY_ERROR_CLASS);
	u2 memberId = getMemberBuildinDependecy(OUT_OF_MEMORY_ERROR_GET_INSTANCE);
	call_static_method(classId, memberId);

	// Pop exception:
	jobject exception = operandStackPopObjectRef();

	throwException(exception);
}

INSTRUCTION(f_athrow) {
	// pop exception:
	jobject exception = operandStackPopObjectRef();
	if (exception == NULL) {
		throwNullPointerException();
	}
	else {
    	throwException(exception);
	}
}

INSTRUCTION(f_monitorenter) {
	consout("f_monitorenter: not implemented\n");
}

INSTRUCTION(f_monitorexit) {
	consout("f_monitorexit: not implemented\n");
}

INSTRUCTION(f_pop) {
	pop();
}

INSTRUCTION(f_nop) {
	// No-op!
}

INSTRUCTION(f_new) {
	BEGIN;
	u2 constantPoolIndex = getU2FromCode();
	u2 classId;
	getClassReference(constantPoolIndex, &classId);

	CALL(jobject jref = newObject(classId));

	if (jref != NULL) {
		operandStackPushObjectRef(jref);
	}
	END;
}

/**
 * This method test if S is 'instanceof' T
 * \param classId_S The class id of S
 * \param classId_T The class id of T
 * \return true, if S is 'instanceof' T; false otherwise.
 */
static BOOL isInstanceOf(u2 classId_S, u2 classId_T) {
	CLASS_TYPE type_T = getClassType(classId_T);
	CLASS_TYPE type_S = getClassType(classId_S);

	BOOL instanceOf;
	if (isObjectArray(classId_S) || isPrimitiveValueArray(classId_S)) {
		if (isObjectArray(classId_T)) {
			if (isObjectArray(classId_S)) {
				// Both are object arrays:
				u2 classId_SC = getArrayElementClassId(classId_S);
				u2 classId_TC = getArrayElementClassId(classId_T);
				instanceOf = isInstanceOf(classId_SC, classId_TC);
			} else {
				instanceOf = FALSE;
			}
		} else if (isPrimitiveValueArray(classId_T)) {
			instanceOf = (classId_T == classId_S) ? TRUE : FALSE;
		} else if (type_T == CT_CLASS) {
			instanceOf = classId_T == 0 ? TRUE : FALSE;
		} else {
			// S shall implement T:
			if (!is_S_implementing_T(classId_S, classId_T)) {
				instanceOf = FALSE;
			} else {
				instanceOf = TRUE;
			}
			consout("not tested, since arrays does not implement any interfaces ... %d\n",
					instanceOf);
			jvmexit(1);
		}
	} else if (type_S == CT_CLASS) {
		if (type_T == CT_CLASS) {
			instanceOf = classId_S == classId_T ? TRUE : FALSE;
			if (!instanceOf) {
				instanceOf = is_S_SubClassing_T(classId_S, classId_T);
			}
		} else {
			// S shall implement T:
			if (!is_S_implementing_T(classId_S, classId_T)) {
				instanceOf = FALSE;
			} else {
				instanceOf = TRUE;
			}
		}
	} else if (type_S == CT_INTERFACE) {
		// All objects in this VM has a class reference; so we don't support any classes having a class id which
		// is an interface.
		// This piece of code is not testable nor tested ;-)
		consout("not implemented\n");
		jvmexit(1);
	} else {
		consout("not implemented\n");
		jvmexit(1);
	}

	/*
	 *i If S is an ordinary (nonarray) class, then:
	 *i    o If T is a class type, then S must be the same class (§2.8.1) as T or a subclass of T.
	 *i    o If T is an interface type, then S must implement (§2.13) interface T.
	 *i If S is an interface type, then:
	 *-    o If T is a class type, then T must be Object (§2.4.7).
	 *-    o If T is an interface type, then T must be the same interface as S, or a superinterface of S (§2.13.2).
	 *  If S is a class representing the array type SC[], that is, an array of components of type SC, then:
	 *i    o If T is a class type, then T must be Object (§2.4.7).
	 *i    o If T is an array type TC[], that is, an array of components of type TC, then one of the following must be true:
	 *i        + TC and SC are the same primitive type (§2.4.1).
	 *i        + TC and SC are reference types (§2.4.6), and type SC can be cast to TC by these runtime rules.
	 *(i)  o If T is an interface type, T must be one of the interfaces implemented by arrays (§2.15).
	 */
	return instanceOf;
}

INSTRUCTION(f_checkcast) {
	u2 constantPoolIndex = getU2FromCode();
	u2 classId_T;
	getClassReference(constantPoolIndex, &classId_T);

	// Get a copy of the item at the top of the stack:
	stackable* ref = pop();
	push(ref->operand, ref->type);

	if (ref->type != OBJECTREF) {
		consout("OBJECTREF expected; got %d\n", ref->type);
		jvmexit(1);
	}

	if (ref->operand.jref != NULL) {
		u2 classId_S = getClassIdFromObject(ref->operand.jref);

		if (!isInstanceOf(classId_S, classId_T)) {
			throwClassCastException(classId_S, classId_T);
		}
	}
	//else: null is OK
}

INSTRUCTION(f_instanceof) {
	u2 constantPoolIndex = getU2FromCode();
	u2 classId_T;
	getClassReference(constantPoolIndex, &classId_T);

	// Pop off the item at the top of the stack:
	stackable* ref = pop();

	if (ref->type != OBJECTREF) {
		consout("OBJECTREF expected; got %d\n", ref->type);
		jvmexit(1);
	}

	jint instanceOf;

	if (ref->operand.jref != NULL) {
		u2 classId_S = getClassIdFromObject(ref->operand.jref);
		instanceOf = isInstanceOf(classId_S, classId_T) ? 1 : 0;
	} else {
		instanceOf = 0;
	}

	operandStackPushJavaInt(instanceOf);
}
INSTRUCTION(f_ishl) {
	jint jint2 = operandStackPopJavaInt() & 0x1f;
	jint jint1 = operandStackPopJavaInt();
	operandStackPushJavaInt(jint1 << jint2);
}

INSTRUCTION(f_ishr) {
	jint jint2 = operandStackPopJavaInt() & 0x1f;
	jint jint1 = operandStackPopJavaInt();
	operandStackPushJavaInt(jint1 >> jint2);
}

INSTRUCTION(f_newarray) {
	BEGIN;
	ARRAY_TYPE type = (ARRAY_TYPE) getU1FromCode();
	jint count = operandStackPopJavaInt();

	if (count < 0) {
		throwNegativeArraySizeException();
	} else {
		// Simple type array allocation:
		u2 classId;

		CLASS_TYPE ct = convertArrayType(type);
		classId = getClassIdForClassType(ct);

		CALL(jobject jref = heapAllocPrimitiveTypeArray(count, type, classId));
		if (jref != NULL) {
			operandStackPushObjectRef(jref);
		}
		// else: Out of mem has been thrown
	}
	END;
}

INSTRUCTION(f_anewarray) {
	BEGIN;

	u2 constantPoolIndex = getU2FromCode();
	u2 elementClassId;
	getClassReference(constantPoolIndex, &elementClassId);
	jint count = operandStackPopJavaInt();
	if (count < 0) {
		throwNegativeArraySizeException();
	} else {

		CALL(jobject jref = heapAllocObjectArray(count, elementClassId));
		if (jref != NULL) {
			operandStackPushObjectRef(jref);
		}
		// else: Out of mem has been thrown
	}
	END;
}

static BOOL validateArrayRef(jint ix, array* ref) {
	BOOL refOk = TRUE;
	if (ref == NULL) {
		//__DEBUG("throwing NPE... %p\n", ref);
		throwNullPointerException();
		refOk = FALSE;
	} else if (ix < 0 || ix >= ref->header.length) {
		throwArrayIndexOutOfBoundsException(ix, ref->header.length);
		DEB(consout("IndexOutOfBoundsException: %d %d", ix, ref->header.length));
		refOk = FALSE;
	}

	return refOk;
}

//
// This macro will reference an array element. NullPointerException and ArrayIndexOutOfBoundsException is
// thrown if conditions are met.
//
// Note! Always test on 'arrayRefOk' before continuing processing!
//
#define ARRAY_REF(IX, REF) \
		jint IX = operandStackPopJavaInt();\
		array* REF = (array*) operandStackPopObjectRef();\
	    BOOL arrayRefOk = validateArrayRef(IX, REF);

INSTRUCTION(f_aaload) {
	// Note! This instruction and aastore might corrupt the stack if the 'array' - struct isn't proper
	// aligned; see also comment in heap.h regarding 'array' struct.
	ARRAY_REF(index, a);
	if (arrayRefOk) {
		jobject* jref = (jobject*) (&a->data[index * sizeof(jobject)]);
		operandStackPushObjectRef(*jref);
	}
}

void heapTester(void);

INSTRUCTION(f_aastore) {
	jobject value = operandStackPopObjectRef();
	ARRAY_REF(index, a);
	if (arrayRefOk) {
		jobject* p = (jobject*) (&a->data[index * sizeof(jobject)]);
		*p = value;
	}
}

INSTRUCTION(f_baload) {
	ARRAY_REF(index, a);

	if (arrayRefOk) {
		jint value;
		heapElement* hep = OBJECT_REF_TO_HEAP_ELEMENT(a);

		CLASS_TYPE ctype = getClassType(hep->classId);

		if (ctype == CT_BOOLEAN_ARRAY) {
			// A boolean occupies a single byte:
			javabyte* jp = (javabyte*) (&a->data[index * sizeof(javabyte)]);
			value = (*jp & 0x01);
		} else if (ctype == CT_BYTE_ARRAY) {
			javabyte* jp = (javabyte*) (&a->data[index * sizeof(javabyte)]);
			value = (int) *jp;
		} else {
			consout("Wrong class(array) type: %d", ctype);
			jvmexit(1);
		}

		operandStackPushJavaInt(value);
	}
}

INSTRUCTION(f_bastore) {
	jint value = operandStackPopJavaInt();
	ARRAY_REF(index, a);

	if (arrayRefOk) {
		heapElement* hep = OBJECT_REF_TO_HEAP_ELEMENT(a);

		CLASS_TYPE ctype = getClassType(hep->classId);

		if (ctype == CT_BOOLEAN_ARRAY) {
			// A boolean occupies a single byte:
			javabyte* jp = (javabyte*) (&a->data[index * sizeof(javabyte)]);
			*jp = (s1) (value & 0x01);
		} else if (ctype == CT_BYTE_ARRAY) {
			javabyte* jp = (javabyte*) (&a->data[index * sizeof(javabyte)]);
			*jp = (s1) (value & 0xff);
		} else {
			consout("Wrong class(array) type: %d", ctype);
			jvmexit(1);
		}
	}
}

INSTRUCTION(f_caload) {
	ARRAY_REF(index, a);
	if (arrayRefOk) {
		jchar* jp = (jchar*) (&a->data[index * sizeof(jchar)]);
		operandStackPushJavaInt(*jp);
	}
}

INSTRUCTION(f_iaload) {
	ARRAY_REF(index, a);
	if (arrayRefOk) {
		jint* jp = (jint*) (&a->data[index * sizeof(jint)]);
		operandStackPushJavaInt(*jp);
	}
}

INSTRUCTION(f_iastore) {
	jint value = operandStackPopJavaInt();
	ARRAY_REF(index, a);
	if (arrayRefOk) {
		jint* jp = (jint*) (&a->data[index * sizeof(jint)]);
		*jp = value;
	}
}

INSTRUCTION(f_castore) {
	jint value = operandStackPopJavaInt();
	ARRAY_REF(index, a);
	if (arrayRefOk) {
		jchar* jp = (jchar*) (&a->data[index * sizeof(jchar)]);

		*jp = value;
	}
}

INSTRUCTION(f_arraylength) {
	array* a = (array*) operandStackPopObjectRef();
	if (a == NULL) {
		throwNullPointerException();
	} else {
		operandStackPushJavaInt(a->header.length);
	}
}

INSTRUCTION(f_iinc) {
	u1 varnum = getU1FromCode();
	jint delta = (jint) getS1FromCode();

	operandStackIncrementVariableJavaInt(varnum, delta);
}

INSTRUCTION(f_unused) {
	DEB(consout("Unsupported instruction"));
	jvmexit(1);
}

INSTRUCTION(f_dup) {
	stackable* st = pop();
	push(st->operand, st->type);
	push(st->operand, st->type);
}

INSTRUCTION(f_dup2) {
	stackable* st2 = pop();
	stackable* st1 = pop();
	push(st1->operand, st1->type);
	push(st2->operand, st2->type);
	push(st1->operand, st1->type);
	push(st2->operand, st2->type);
}

INSTRUCTION(f_bipush) {
	// Sign extend:
	s1 val = getU1FromCode();
	operandStackPushJavaInt((jint) val);
}

INSTRUCTION(f_sipush) {
	s2 val = getS2FromCode();
	operandStackPushJavaInt((jint) val);
}

INSTRUCTION(f_i2b) {
	jint val = operandStackPopJavaInt();
	val = val & 0xff;
	if ((val & 0xffffff80) != 0) {
		// Sign extend:
		val |= 0xffffff00;
	}
	operandStackPushJavaInt(val);
}

/**
 * This method creates and allocates a java.lang.String object and returns the created Object
 * \param value The constant string to initiate with
 * \return The allocated object
 */
static jobject allocJavaLangString(const char* value) {
	BEGIN;
	//markAndSweep();
	//printf("*** ALLOC STRING BEGIN ***\n");
	// Allocate String object:
	u2 size;
	getClassSize(javaLangStringClassIndex, &size); // size of java.lang.String
	CALL(jobject stringObject = heapAllocObject(size, javaLangStringClassIndex));

	if (stringObject != NULL) {
		// Allocate char[]:
		size_t len = strlen(value);

		// Avoid garbage collection of the newly allocated String object (there is no reference to it yet,
		// so a protection is necessary):
		CALL(heapProtect(stringObject));
		u2 charArrayClassId = getClassIdForClassType(CT_CHAR_ARRAY);
		CALL(jobject charArray = heapAllocPrimitiveTypeArray(len, 1, charArrayClassId));
		CALL(heapProtect(NULL));

		if (charArray != NULL) {
			// memcpy into charArray (length and type are set):
			int i;
			array* a = (array*) charArray;
			for (i = 0; i < len; i++) {
				a->data[i] = value[i];
			}

			stackable* val = (stackable*) stringObject;
			// TODO: This is a hack - we are exploiting that the char[] is stackable[0] in a String!
			val->operand.jref = charArray + (0 * sizeof(stackable));
			val->type = OBJECTREF;
		}
		// else: out of mem has been thrown
	}
	END;
	return stringObject;
}

INSTRUCTION(f_ldc) {
	u1 constantPoolIndex = getU1FromCode();
	// Look up value (int, float or String) from within const pool:
	constantDef constant;
	getConstant(constantPoolIndex, &constant);

	if (constant.type == CONSTANT_INTEGER) {
		operandStackPushJavaInt(constant.value.jrenameint);
	} else if (constant.type == CONSTANT_STRING) {
		jobject str = allocJavaLangString(constant.value.string);
		operandStackPushObjectRef(str);
	} else if (constant.type == CONSTANT_CLASS) {
		//registerNatives skal bygge det Class[], der skal foretages lookup i:
		jclass jc = getJavaLangClass(constant.value.classId);
		operandStackPushObjectRef(jc);
	} else {
		// TODO Support float
		consout("Unsupported type: %d\n", constant.type);
		jvmexit(1);
	}
}

INSTRUCTION(f_vreturn) {
	BEGIN;
	pop_frame();
	END;
}

INSTRUCTION(f_invokeinterface) {
	BEGIN;
	u2 methodRef = getU2FromCode();
	getU1FromCode(); // 'count'
	getU1FromCode(); // shall always be 0 - but who cares ?!

	//---------------------------------------------------------------
	const methodInClass* mic;

	CALL(mic = getVirtualMethodEntry(methodRef));

	if (mic != NULL) {
		CALL(invokeCommon(mic, FALSE))
		;
	}
	//---------------------------------------------------------------
	END;
}

INSTRUCTION(f_invokevirtual) {
	BEGIN;
	u2 methodRef = getU2FromCode();

	//---------------------------------------------------------------
	const methodInClass* mic;

	CALL(mic = getVirtualMethodEntry(methodRef));

	if (mic != NULL) {
		CALL(invokeCommon(mic, FALSE))
		;
	}
	//---------------------------------------------------------------
	END;
}

INSTRUCTION(f_invokespecial) {
	BEGIN;
	u2 methodRef = getU2FromCode();

	const methodInClass* mic;

	CALL(mic = getStaticMethodEntry(context.classIndex, methodRef));

	if (operandStackIsObjectRefAtOffsetNull(mic->numberOfArguments)) {
		throwNullPointerException();
	} else {
		CALL(invokeCommon(mic, FALSE))
		;
	}
	END;
}

void invoke_static(u2 staticMethodRef) {
	BEGIN;

	const methodInClass* mic;

	CALL(mic = getStaticMethodEntry(context.classIndex, staticMethodRef));

	CALL(invokeCommon(mic, FALSE));

	END;
}

INSTRUCTION(f_invokestatic) {
	BEGIN;

	u2 staticMethodRef = getU2FromCode();

	CALL(invoke_static(staticMethodRef));

	END;
}

INSTRUCTION(f_putstatic) {
	u2 staticFieldRef = getU2FromCode();
	u2 address;
	u1 size;
	getStaticFieldEntry(staticFieldRef, &address, &size);
	if (size != 1) {
		// TODO support
		DEB(consout("No support for size != 1 (double, long, void): %d\n", size));
		jvmexit(1);
	}
	stackable* value = pop();
	staticMemory[address] = *value;
}

INSTRUCTION(f_getstatic) {
	BEGIN;
	u2 staticFieldRef = getU2FromCode();
	u2 address;
	u1 size;
	getStaticFieldEntry(staticFieldRef, &address, &size);
	if (size != 1) {
		DEB(consout("No support for size != 1 (double, long, void): %d\n", size));
		jvmexit(1);
	}
	stackable value = staticMemory[address];
	push(value.operand, value.type);
	//CALL(validateStackables(stack, context.operandStackPointer));

	END;
}

INSTRUCTION(f_putfield) {
	u2 fieldRef = getU2FromCode();
	u2 address;
	u1 size;
	// putfield is relative to 'this', which is pushed onto the stack prior to this instruction
	getInstanceFieldEntry(fieldRef, &address, &size);

	if (size != 1) {
		// TODO support
		DEB(consout("No support for size != 1 (double, long, void): %d\n", size));
		jvmexit(1);
	}
	stackable* value = pop();
	stackable* this = pop();
	if (this->type != OBJECTREF) {
		DEB(consout("Type mismatch: Expected OBJECTREF, got %d\n", this->type));
		jvmexit(1);
	}

	if (this->operand.jref != NULL) {
		*((stackable*) (((void*) (this->operand.jref)) + address * sizeof(stackable))) = *value;
	} else {
		throwNullPointerException();
	}
}

INSTRUCTION(f_getfield) {
	u2 fieldRef = getU2FromCode();
	u2 address;
	u1 size;
	getInstanceFieldEntry(fieldRef, &address, &size);
	if (size != 1) {
		DEB(consout("No support for size != 1 (double, long, void): %d\n", size));
		jvmexit(1);
	}
	stackable* this = pop();
	if (this->operand.jref == NULL) {
		throwNullPointerException();
	} else {
		stackable* value = (stackable*) ((void*) (this->operand.jref) + address * sizeof(stackable));

		//	printf("getfield: ref = %d, address = %d, this = %ld\n", fieldRef, address, this.operand.jref);
		//	printf("getfield: type = %d\n", value->type);
		push(value->operand, value->type);
	}
}

INSTRUCTION(f_areturn) {
	BEGIN;

	// Object return value:
	jobject jref = operandStackPopObjectRef();

	f_vreturn();

	// Push result:
	operandStackPushObjectRef(jref);
	END;
}

INSTRUCTION(f_ireturn) {
	// Integer return value:
	jint jrenameint = operandStackPopJavaInt();

	f_vreturn();

	// Push result:
	operandStackPushJavaInt(jrenameint);
}

INSTRUCTION(f_aload) {
	u1 index = getU1FromCode();
	operandStackPushVariableObjectRef(index);
}

INSTRUCTION(f_aload_0) {
	operandStackPushVariableObjectRef(0);
}

INSTRUCTION(f_aload_1) {
	operandStackPushVariableObjectRef(1);
}

INSTRUCTION(f_aload_2) {
	operandStackPushVariableObjectRef(2);
}

INSTRUCTION(f_aload_3) {
	operandStackPushVariableObjectRef(3);
}

INSTRUCTION(f_iload) {
	u1 index = getU1FromCode();
	operandStackPushVariableJavaInt(index);
}

INSTRUCTION(f_iload_0) {
	operandStackPushVariableJavaInt(0);
}

INSTRUCTION(f_iload_1) {
	operandStackPushVariableJavaInt(1);
}

INSTRUCTION(f_iload_2) {
	operandStackPushVariableJavaInt(2);
}

INSTRUCTION(f_iload_3) {
	operandStackPushVariableJavaInt(3);
}

#define IFINS(NAME, CONDITION) \
		INSTRUCTION(NAME) { \
			s2 offset = getS2FromCode();\
			\
			if (CONDITION) { \
				context.programCounter -= 3;\
				context.programCounter += offset; \
			} \
		}

IFINS(f_ifeq, operandStackPopJavaInt() == 0)
IFINS(f_ifne, operandStackPopJavaInt() != 0)
IFINS(f_ifgt, operandStackPopJavaInt() > 0)
IFINS(f_ifle, operandStackPopJavaInt() <= 0)
IFINS(f_iflt, operandStackPopJavaInt() < 0)
IFINS(f_ifge, operandStackPopJavaInt() >= 0)
IFINS(f_if_icmpeq, operandStackPopJavaInt() == operandStackPopJavaInt())
IFINS(f_if_icmpne, operandStackPopJavaInt() != operandStackPopJavaInt())
IFINS(f_if_acmpeq, operandStackPopObjectRef() == operandStackPopObjectRef())
IFINS(f_if_acmpne, operandStackPopObjectRef() != operandStackPopObjectRef())
// Note! Left is pushed first and popped last => the condition shall be 'inverted'
IFINS(f_if_icmple, operandStackPopJavaInt() >= operandStackPopJavaInt())
IFINS(f_if_icmplt, operandStackPopJavaInt() > operandStackPopJavaInt())
IFINS(f_if_icmpge, operandStackPopJavaInt() <= operandStackPopJavaInt())
IFINS(f_if_icmpgt, operandStackPopJavaInt() < operandStackPopJavaInt())
//INSTRUCTION(if_icmple) {
//	s2 offset = getS2FromCode();
//
//	javaint value1 = operandStackPopJavaInt();
//	javaint value2 = operandStackPopJavaInt();
//	printf("val1 = %d, val2 = %d\n", value1, value2);
//	if (value1 <= value2) {
//		context.programCounter -= 3;
//		context.programCounter += offset;
//	}
//}
IFINS(f_ifnonnull, operandStackPopObjectRef() != NULL)
IFINS(f_ifnull, operandStackPopObjectRef() == NULL)

IFINS(f_goto, TRUE)
;

INSTRUCTION(f_aconst_null) {
	operandStackPushObjectRef((jobject) NULL);
}

INSTRUCTION(f_iconst_m1) {
	operandStackPushJavaInt(-1);
}

INSTRUCTION(f_iconst_0) {
	operandStackPushJavaInt(0);
}

INSTRUCTION(f_iconst_1) {
	operandStackPushJavaInt(1);
}

INSTRUCTION(f_iconst_2) {
	operandStackPushJavaInt(2);
}

INSTRUCTION(f_iconst_3) {
	operandStackPushJavaInt(3);
}

INSTRUCTION(f_iconst_4) {
	operandStackPushJavaInt(4);
}

INSTRUCTION(f_iconst_5) {
	operandStackPushJavaInt(5);
}

INSTRUCTION(f_isub) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 - jrenameint1);
}

INSTRUCTION(f_iand) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 & jrenameint1);
}

INSTRUCTION(f_ior) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 | jrenameint1);
}

INSTRUCTION(f_ixor) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 ^ jrenameint1);
}

INSTRUCTION(f_iadd) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 + jrenameint1);
}

static const char* divisionByZeroMessage = "Division by zero";
INSTRUCTION(f_idiv) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	if (jrenameint1 == 0) {
		throwArithmeticException(divisionByZeroMessage);
	} else {
		operandStackPushJavaInt(jrenameint2 / jrenameint1);
	}
}

INSTRUCTION(f_ineg) {
	operandStackPushJavaInt(-operandStackPopJavaInt());
}

INSTRUCTION(f_irem) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	if (jrenameint1 == 0) {
		throwArithmeticException(divisionByZeroMessage);
	} else {
		operandStackPushJavaInt(jrenameint2 % jrenameint1);
	}
}

INSTRUCTION(f_imul) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 * jrenameint1);
}

INSTRUCTION(f_astore) {
	u1 index = getU1FromCode();
	operandStackPopVariableObjectRef(index);
}

INSTRUCTION(f_astore_0) {
	operandStackPopVariableObjectRef(0);
}

INSTRUCTION(f_astore_1) {
	operandStackPopVariableObjectRef(1);
}

INSTRUCTION(f_astore_2) {
	operandStackPopVariableObjectRef(2);
}

INSTRUCTION(f_astore_3) {
	operandStackPopVariableObjectRef(3);
}

INSTRUCTION(f_istore) {
	u1 index = getU1FromCode();
	operandStackPopVariableJavaInt(index);
}

INSTRUCTION(f_istore_0) {
	operandStackPopVariableJavaInt(0);
}

INSTRUCTION(f_istore_1) {
	operandStackPopVariableJavaInt(1);
}

INSTRUCTION(f_istore_2) {
	operandStackPopVariableJavaInt(2);
}

INSTRUCTION(f_istore_3) {
	operandStackPopVariableJavaInt(3);
}

//INSTRUCTION(f_halt) {
//	// Garbage collect:
//	markAndSweep();
//
//	consout("********************************\n");
//	consout("HALT instruction met; halting VM\n");
//	consout("********************************\n");
//
//	jvmexit(0);
//}

//00 (0x00) nop
//
//01 (0x01) aconst_null
//
//02 (0x02) iconst_m1
//
//03 (0x03) iconst_0
//
//04 (0x04) iconst_1
//
//05 (0x05) iconst_2
//
//06 (0x06) iconst_3
//
//07 (0x07) iconst_4
//
//08 (0x08) iconst_5
//
//09 (0x09) lconst_0
//
//10 (0x0a) lconst_1
//
//11 (0x0b) fconst_0
//
//12 (0x0c) fconst_1
//
//13 (0x0d) fconst_2
//
//14 (0x0e) dconst_0
//
//15 (0x0f) dconst_1
//
//16 (0x10) bipush
//
//17 (0x11) sipush
//
//18 (0x12) ldc
//
//19 (0x13) ldc_w
//
//20 (0x14) ldc2_w
//
//21 (0x15) iload
//
//22 (0x16) lload
//
//23 (0x17) fload
//
//24 (0x18) dload
//
//25 (0x19) aload
//
//26 (0x1a) iload_0
//
//27 (0x1b) iload_1
//
//28 (0x1c) iload_2
//
//29 (0x1d) iload_3
//
//30 (0x1e) lload_0
//
//31 (0x1f) lload_1
//
//32 (0x20) lload_2
//
//33 (0x21) lload_3
//
//34 (0x22) fload_0
//
//35 (0x23) fload_1
//
//36 (0x24) fload_2
//
//37 (0x25) fload_3
//
//38 (0x26) dload_0
//
//39 (0x27) dload_1
//
//40 (0x28) dload_2
//
//41 (0x29) dload_3
//
//42 (0x2a) aload_0
//
//43 (0x2b) aload_1
//
//44 (0x2c) aload_2
//
//45 (0x2d) aload_3
//
//46 (0x2e) iaload
//
//47 (0x2f) laload
//
//48 (0x30) faload
//
//49 (0x31) daload
//
//50 (0x32) aaload
//
//51 (0x33) baload
//
//52 (0x34) caload
//
//53 (0x35) saload
//
//54 (0x36) istore
//
//55 (0x37) lstore
//
//56 (0x38) fstore
//
//57 (0x39) dstore
//
//58 (0x3a) astore
//
//59 (0x3b) istore_0
//
//60 (0x3c) istore_1
//
//61 (0x3d) istore_2
//
//62 (0x3e) istore_3
//
//63 (0x3f) lstore_0
//
//64 (0x40) lstore_1
//
//65 (0x41) lstore_2
//
//66 (0x42) lstore_3
//
//67 (0x43) fstore_0
//
//68 (0x44) fstore_1
//
//69 (0x45) fstore_2
//
//70 (0x46) fstore_3
//
//71 (0x47) dstore_0
//
//72 (0x48) dstore_1
//
//73 (0x49) dstore_2
//
//74 (0x4a) dstore_3
//
//75 (0x4b) astore_0
//
//76 (0x4c) astore_1
//
//77 (0x4d) astore_2
//
//78 (0x4e) astore_3
//
//79 (0x4f) iastore
//
//80 (0x50) lastore
//
//81 (0x51) fastore
//
//82 (0x52) dastore
//
//83 (0x53) aastore
//
//84 (0x54) bastore
//
//85 (0x55) castore
//
//86 (0x56) sastore
//
//87 (0x57) pop
//
//88 (0x58) pop2
//
//089 (0x59) dup
//
//090 (0x5a) dup_x1
//
//091 (0x5b) dup_x2
//
//092 (0x5c) dup2
//
//093 (0x5d) dup2_x1
//
//094 (0x5e) dup2_x2
//
//095 (0x5f) swap
//
//096 (0x60) iadd
//
//097 (0x61) ladd
//
//098 (0x62) fadd
//
//099 (0x63) dadd
//
//100 (0x64) isub
//
//101 (0x65) lsub
//
//102 (0x66) fsub
//
//103 (0x67) dsub
//
//104 (0x68) imul
//
//105 (0x69) lmul
//
//106 (0x6a) fmul
//
//107 (0x6b) dmul
//
//108 (0x6c) idiv
//
//109 (0x6d) ldiv
//
//110 (0x6e) fdiv
//
//111 (0x6f) ddiv
//
//112 (0x70) irem
//
//113 (0x71) lrem
//
//114 (0x72) frem
//
//115 (0x73) drem
//
//116 (0x74).......ineg
//
//117 (0x75) lneg
//
//118 (0x76) fneg
//
//119 (0x77) dneg
//
//120 (0x78) ishl
//
//121 (0x79) lshl
//
//122 (0x7a) ishr
//
//123 (0x7b) lshr
//
//124 (0x7c) iushr
//
//125 (0x7d) lushr
//
//126 (0x7e) iand
//
//127 (0x7f) land
//
//128 (0x80) ior
//
//129 (0x81) lor
//
//130 (0x82) ixor
//
//131 (0x83) lxor
//
//132 (0x84) iinc
//
//133 (0x85) i2l
//
//134 (0x86) i2f
//
//135 (0x87) i2d
//
//136 (0x88) l2i
//
//137 (0x89) l2f
//
//138 (0x8a) l2d
//
//139 (0x8b) f2i
//
//140 (0x8c) f2l
//
//141 (0x8d) f2d
//
//142 (0x8e) d2i
//
//143 (0x8f) d2l
//
//144 (0x90) d2f
//
//145 (0x91) i2b
//
//146 (0x92) i2c
//
//147 (0x93) i2s
//
//148 (0x94) lcmp
//
//149 (0x95) fcmpl
//
//150 (0x96) fcmpg
//
//151 (0x97) dcmpl
//
//152 (0x98) dcmpg
//
//153 (0x99) ifeq
//
//154 (0x9a) ifne
//
//155 (0x9b) iflt
//
//156 (0x9c) ifge
//
//157 (0x9d) ifgt
//
//158 (0x9e) ifle
//
//159 (0x9f) if_icmpeq
//
//160 (0xa0) if_icmpne
//
//161 (0xa1) if_icmplt
//
//162 (0xa2) if_icmpge
//
//163 (0xa3) if_icmpgt
//
//164 (0xa4) if_icmple
//
//165 (0xa5) if_acmpeq
//
//166 (0xa6) if_acmpne
//
//167 (0xa7) goto
//
//168 (0xa8) jsr
//
//169 (0xa9) ret
//
//170 (0xaa) tableswitch
//
//171 (0xab) lookupswitch
//
//172 (0xac) ireturn
//
//173 (0xad) lreturn
//
//174 (0xae) freturn
//
//175 (0xaf) dreturn
//
//176 (0xb0) areturn
//
//177 (0xb1) return
//
//178 (0xb2) getstatic
//
//179 (0xb3) putstatic
//
//180 (0xb4) getfield
//
//181 (0xb5) putfield
//
//182 (0xb6) invokevirtual
//
//183 (0xb7) invokespecial
//
//184 (0xb8) invokestatic
//
//185 (0xb9) invokeinterface
//
//186 (0xba) xxxunusedxxx1
//
//187 (0xbb) new
//
//188 (0xbc) newarray
//
//189 (0xbd) anewarray
//
//190 (0xbe) arraylength
//
//191 (0xbf) athrow
//
//192 (0xc0) checkcast
//
//193 (0xc1) instanceof
//
//194 (0xc2) monitorenter
//
//195 (0xc3) monitorexit
//
//196 (0xc4) wide
//
//197 (0xc5) multianewarray
//
//198 (0xc6) ifnull
//
//199 (0xc7) ifnonnull
//
//200 (0xc8) goto_w
//
//201 (0xc9) jsr_w
//
//Reserved opcodes:
//
//202 (0xca) breakpoint
//
//254 (0xfe) impdep1
//
//255 (0xff) impdep2

