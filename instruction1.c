/*
 * instruction.c
 *
 *  Created on: Aug 21, 2010
 *      Author: hammer
 */
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "jarray.h"
#include "console.h"
#include "frame.h"
#include "heap.h"
#include "heaplist.h"
#include "instructions.h"
#include "debugger.h"
#include "trace.h"
#include "jni.h"
#include "exceptions.h"
#include "objectaccess.h"

extern stackable staticMemory[];

static const char* divisionByZeroMessage = "Division by zero";

static void inUndefinedInstruction(const char* file, int line) {
	consout("%s:%d: Undefined instruction at 0x%04x\n", file, line, context.programCounter - 1);
	jvmexit(1);
}

void execute() {
	// Instantiate static jump table:
	static JUMP_TABLE;

	/////////////////////////////////////////////////////////
	// This is the point to go to when a new instruction shall be executed:
	/////////////////////////////////////////////////////////
	nextInstruction:

	HEAP_VALIDATE;
	tryYield();
	HEAP_VALIDATE;

	if (context.flags & RETURN_FROM_VM) {
		// Clear return flag: this is a one-shot:
		context.flags &= ~RETURN_FROM_VM;
		return;
	}

	context.exceptionThrown = FALSE;
	// Note! 'code' is u1[] and we have 256 entries in the array => no check needed
	goto *jumpTable[code[context.programCounter++]];

	/////////////////////////////////////////////////////////
	// Instructions BEGIN
	/////////////////////////////////////////////////////////


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

	IFINS(f_ifnonnull, operandStackPopObjectRef() != NULL)
	IFINS(f_ifnull, operandStackPopObjectRef() == NULL)

	IFINS(f_goto, TRUE)
	//
	//
	INS_BEGIN(f_athrow){
	// pop exception:
	jobject exception = operandStackPopObjectRef();
	if (exception == NULL) {
		throwNullPointerException();
	} else {
		throwException(exception);
	}
}
//
INS_END

INS_BEGIN(f_monitorenter) {
	consout("f_monitorenter: not implemented\n");
}
INS_END

INS_BEGIN(f_monitorexit) {
	consout("f_monitorexit: not implemented\n");
}
INS_END

INS_BEGIN(f_pop) {
	stackable op;
	pop(&op);
}
INS_END

INS_BEGIN(f_pop2) {
	// When pop'ing we exploit that a category 2 consists of two entries on the stack:
	stackable op;
	pop(&op);
	pop(&op);
}
INS_END

INS_BEGIN(f_nop)
// No-op!
INS_END

INS_BEGIN(f_new) {
	u2 constantPoolIndex = getU2FromCode();
	u2 classId;
	getClassReference(constantPoolIndex, &classId);

	jobject jref = newObject(classId);

	if (jref != NULL) {
		operandStackPushObjectRef(jref);
	}
}
INS_END

INS_BEGIN(f_checkcast) {
	u2 constantPoolIndex = getU2FromCode();
	u2 classId_T;
	getClassReference(constantPoolIndex, &classId_T);

	// Get a copy of the item at the top of the stack:
	stackable ref;
	pop(&ref);
	push(ref.operand, ref.type);

	VALIDATE_TYPE(ref.type , OBJECTREF)

	if (ref.operand.jref != NULL) {
		u2 classId_S = oaGetClassIdFromObject(ref.operand.jref);

		if (!CP_IsInstanceOf(classId_S, classId_T)) {
			throwClassCastException(classId_S, classId_T);
		}
	}
	//else: null is OK
}
INS_END

INS_BEGIN(f_instanceof) {
	u2 constantPoolIndex = getU2FromCode();
	u2 classId_T;
	getClassReference(constantPoolIndex, &classId_T);

	// Pop off the item at the top of the stack:
	stackable ref;
	pop(&ref);

	VALIDATE_TYPE(ref.type, OBJECTREF);

	jint instanceOf;

	if (ref.operand.jref != NULL) {
		u2 classId_S = oaGetClassIdFromObject(ref.operand.jref);
		instanceOf = CP_IsInstanceOf(classId_S, classId_T) ? 1 : 0;
	} else {
		instanceOf = 0;
	}

	operandStackPushJavaInt(instanceOf);
}
INS_END

INS_BEGIN(f_ishl) {
	jint jint2 = operandStackPopJavaInt() & 0x1f;
	jint jint1 = operandStackPopJavaInt();
	operandStackPushJavaInt(jint1 << jint2);
}
INS_END

INS_BEGIN(f_ishr) {
	jint jint2 = operandStackPopJavaInt() & 0x1f;
	jint jint1 = operandStackPopJavaInt();
	operandStackPushJavaInt(jint1 >> jint2);
}
INS_END

INS_BEGIN(f_lshl) {
	jint v2 = operandStackPopJavaInt() & 0x3f;
	jlong v1 = operandStackPopJavaLong();
	operandStackPushJavaLong(v1 << v2);
}
INS_END

INS_BEGIN(f_lshr) {
	jint v2 = operandStackPopJavaInt() & 0x3f;
	jlong v1 = operandStackPopJavaLong();
	operandStackPushJavaLong(v1 >> v2);
}
INS_END

INS_BEGIN(f_lushr) {
	jint v2 = operandStackPopJavaInt() & 0x3f;
	jlong v1 = operandStackPopJavaLong();
	operandStackPushJavaLong(((ujlong)v1) >> v2);
}
INS_END

INS_BEGIN(f_newarray) {
	ARRAY_TYPE type = (ARRAY_TYPE) getU1FromCode();
	jint count = operandStackPopJavaInt();

	jobject jobj;

	if (count < 0) {
		throwNegativeArraySizeException();
	} else {
		switch (type) {
			case T_CHAR:
			jobj = NewCharArray(count);
			break;
			case T_INT:
			jobj = NewIntArray(count);
			break;
			case T_BOOLEAN:
			jobj = NewBooleanArray(count);
			break;
			case T_BYTE:
			jobj = NewByteArray(count);
			break;
			case T_LONG:
			jobj = NewLongArray(count);
			break;
			case T_FLOAT:
			case T_DOUBLE:
			case T_SHORT:
			case T_REFERENCE:
			consout("not impl: %d\n", type);
			jvmexit(1);
		}
		if (jobj != NULL) {
			operandStackPushObjectRef(jobj);
		}
		// else: Out of mem has been thrown
	}
}
INS_END

INS_BEGIN(f_anewarray) {
	u2 constantPoolIndex = getU2FromCode();
	u2 elementClassId;
	getClassReference(constantPoolIndex, &elementClassId);
	jint count = operandStackPopJavaInt();
	if (count < 0) {
		throwNegativeArraySizeException();
	} else {
		jobject jref = NewObjectArray(count, elementClassId, NULL);
		if (jref != NULL) {
			operandStackPushObjectRef(jref);
		}
		// else: Out of mem has been thrown
	}
}
INS_END

INS_BEGIN(f_aaload) {
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();
	jobject value = GetObjectArrayElement(a, index);

	if (!ExceptionCheck()) {
		operandStackPushObjectRef(value);
	}
}
INS_END

INS_BEGIN(f_aastore) {
	jobject value = operandStackPopObjectRef();
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();

	SetObjectArrayElement(a, index, value);
}
INS_END

INS_BEGIN(f_baload) {
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();

	if (a == NULL) {
		throwNullPointerException();
	} else {
		jint value;
		CLASS_TYPE ctype = getClassType(oaGetClassIdFromObject(a));

		if (ctype == CT_BOOLEAN_ARRAY) {
			// A boolean occupies a single byte:
			jboolean jp = GetBooleanArrayElement(a, index);
			value = jp ? 1 : 0;
		} else if (ctype == CT_BYTE_ARRAY) {
			value = GetByteArrayElement(a, index);
		} else {
			consout("Wrong class(array) type: %d", ctype);
			jvmexit(1);
		}

		if (!ExceptionCheck()) {
			operandStackPushJavaInt(value);
		}
	}
}
INS_END

INS_BEGIN(f_bastore) {
	jint value = operandStackPopJavaInt();
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();

	if (a == NULL) {
		throwNullPointerException();
	} else {
		CLASS_TYPE ctype = getClassType(oaGetClassIdFromObject(a));

		if (ctype == CT_BOOLEAN_ARRAY) {
			// A boolean occupies a single byte:
			SetBooleanArrayElement(a, index, value & 1);
		} else if (ctype == CT_BYTE_ARRAY) {
			SetByteArrayElement(a, index, value & 0xff);
		} else {
			consout("Wrong class(array) type: %d", ctype);
			jvmexit(1);
		}
	}
}
INS_END

INS_BEGIN(f_caload) {
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();
	jchar value = GetCharArrayElement(a, index);

	if (!ExceptionCheck()) {
		operandStackPushJavaInt(value);
	}
}
INS_END

INS_BEGIN(f_iaload) {
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();
	jint value = GetIntArrayElement(a, index);

	if (!ExceptionCheck()) {
		operandStackPushJavaInt(value);
	}
}
INS_END

INS_BEGIN(f_laload) {
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();
	jlong value = GetLongArrayElement(a, index);

	if (!ExceptionCheck()) {
		operandStackPushJavaLong(value);
	}
}
INS_END

INS_BEGIN(f_iastore) {
	jint value = operandStackPopJavaInt();
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();

	SetIntArrayElement(a, index, value);
}
INS_END

INS_BEGIN(f_lastore) {
	jlong value = operandStackPopJavaLong();
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();

	SetLongArrayElement(a, index, value);
}
INS_END

INS_BEGIN(f_castore) {
	jint value = operandStackPopJavaInt();
	jint index = operandStackPopJavaInt();
	jarray a = (jarray) operandStackPopObjectRef();

	SetCharArrayElement(a, index, value);
}
INS_END

INS_BEGIN(f_arraylength) {
	array_t * a = (array_t*) operandStackPopObjectRef();
	if (a == NULL) {
		throwNullPointerException();
	} else {
		operandStackPushJavaInt(GetArrayLength(a));
	}
}
INS_END

INS_BEGIN(f_iinc) {
	u1 varnum = getU1FromCode();
	jint delta = (jint) getS1FromCode();
	operandStackIncrementVariableJavaInt(varnum, delta);
}
INS_END

INS_BEGIN(f_dup_x1) {
	stackable value1;
	pop(&value1);
	stackable value2;
	pop(&value2);

	push(value1.operand, value1.type);
	push(value2.operand, value2.type);
	push(value1.operand, value1.type);
}
INS_END

INS_BEGIN(f_dup2_x1) {
	// When dup'ing we exploit that a category 2 consists of two entries on the stack:
	//..., value3, value2, value1 => ..., value2, value1, value3, value2, value1
	stackable value1;
	pop(&value1);
	stackable value2;
	pop(&value2);
	stackable value3;
	pop(&value3);

	push(value2.operand, value2.type);
	push(value1.operand, value1.type);

	push(value3.operand, value3.type);

	push(value2.operand, value2.type);
	push(value1.operand, value1.type);
}
INS_END

INS_BEGIN(f_dup) {
	stackable st;
	pop(&st);
	push(st.operand, st.type);
	push(st.operand, st.type);
}
INS_END

INS_BEGIN(f_dup2) {
	stackable st2;
	pop(&st2);
	stackable st1;
	pop(&st1);

	push(st1.operand, st1.type);
	push(st2.operand, st2.type);
	push(st1.operand, st1.type);
	push(st2.operand, st2.type);
}
INS_END

INS_BEGIN(f_bipush) {
	// Sign extend:
	s1 val = getU1FromCode();
	operandStackPushJavaInt((jint) val);
}
INS_END

INS_BEGIN(f_sipush) {
	s2 val = getS2FromCode();
	operandStackPushJavaInt((jint) val);
}
INS_END

INS_BEGIN(f_i2b) {
	jint val = operandStackPopJavaInt();
	val = val & 0xff;
	if ((val & 0xffffff80) != 0) {
		// Sign extend:
		val |= 0xffffff00;
	}
	operandStackPushJavaInt(val);
}
INS_END

INS_BEGIN(f_i2c) {
	jint val = operandStackPopJavaInt();
	val = val & 0xff;
	operandStackPushJavaInt(val);
}
INS_END

INS_BEGIN(f_l2i) {
	jlong val = operandStackPopJavaLong();
	operandStackPushJavaInt((jint)val);
}
INS_END

INS_BEGIN(f_i2l) {
	jint v1 = operandStackPopJavaInt();
	operandStackPushJavaLong((jlong)v1);
}
INS_END

INS_BEGIN(f_ldc) {
	cpCommonLDC(getU1FromCode());
}
INS_END

INS_BEGIN(f_ldc_w) {
	cpCommonLDC(getU2FromCode());
}
INS_END

INS_BEGIN(f_ldc2_w) {
	cpCommonLDC(getU2FromCode());
}
INS_END

INS_BEGIN(f_vreturn) {
	pop_frame();
}
INS_END

INS_BEGIN(f_invokeinterface) {
	u2 methodRef = getU2FromCode();
	getU1FromCode(); // 'count'
	getU1FromCode(); // shall always be 0 - but who cares ?!

	const methodInClass * mic = getVirtualMethodEntry(methodRef);

	if (mic != NULL) {
		invokeCommon(mic, FALSE);
	}
}
INS_END

INS_BEGIN(f_invokevirtual) {
	u2 methodRef = getU2FromCode();

	//---------------------------------------------------------------
	const methodInClass * mic;
	mic = getVirtualMethodEntry(methodRef);

	if (mic != NULL) {
		invokeCommon(mic, FALSE);
	}
	//---------------------------------------------------------------
}
INS_END

INS_BEGIN(f_invokespecial) {
	u2 methodRef = getU2FromCode();

	const methodInClass * mic = getStaticMethodEntry(context.classIndex, methodRef);

	if (osIsObjectRefAtOffsetNull(mic->numberOfArguments)) {
		throwNullPointerException();
	} else {
		invokeCommon(mic, FALSE);
	}
}
INS_END

INS_BEGIN(f_invokestatic) {
	u2 staticMethodRef = getU2FromCode();

	const methodInClass* mic = getStaticMethodEntry(context.classIndex, staticMethodRef);

	invokeCommon(mic, FALSE);
}
INS_END

INS_BEGIN(f_getstatic) {
	u2 staticFieldRef = getU2FromCode();
	u2 address;
	u1 size;

	getStaticFieldEntry(staticFieldRef, &address, &size);

	// MSValue at highest addres; LSValue at lowest:
	stackable value = staticMemory[address];
	push(value.operand, value.type);

	if (size == 2) {
		stackable value = staticMemory[address+1];
		push(value.operand, value.type);
	}
}
INS_END

INS_BEGIN(f_putstatic) {
	u2 staticFieldRef = getU2FromCode();
	u2 address;
	u1 size;
	getStaticFieldEntry(staticFieldRef, &address, &size);
	stackable value;

	// MSValue at highest addres; LSValue at lowest:
	if (size == 2) {
		pop(&value);
		staticMemory[address+1] = value;
	}

	pop(&value);
	staticMemory[address] = value;
}
INS_END

INS_BEGIN(f_putfield) {
	u2 fieldRef = getU2FromCode();
	u2 address;
	u1 size;
	// putfield is relative to 'this', which is pushed onto the stack prior to this instruction
	getInstanceFieldEntry(fieldRef, &address, &size);

	stackable msValue;
	pop(&msValue);
	stackable lsValue;
	if (size == 2) {
		pop(&lsValue);
	}
	stackable this;
	pop(&this);
	VALIDATE_TYPE(this.type, OBJECTREF);

	if (size == 1) {
		PutField(this.operand.jref, address, &msValue);
	} else if (size == 2) {
		PutField(this.operand.jref, address, &lsValue);
		PutField(this.operand.jref, address+1, &msValue);
	} else {
		consout("No support for size != 1 or 2: %d\n", size);
		jvmexit(1);
	}
}
INS_END

INS_BEGIN(f_getfield) {
	u2 fieldRef = getU2FromCode();
	u2 address;
	u1 size;
	//consoutli("getfield: %04x\n", context.programCounter);
	getInstanceFieldEntry(fieldRef, &address, &size);
	stackable this;
	pop(&this);
	if (this.operand.jref != NULL) {
		if (size == 1) {
			stackable * value = GetField(this.operand.jref, address);
			push(value->operand, value->type);
		} else if (size == 2) {
			stackable * value = GetField(this.operand.jref, address);
			push(value->operand, value->type);
			value = GetField(this.operand.jref, address + 1);
			push(value->operand, value->type);
		} else {
			consout("No support for size != 1 or 2: %d\n", size);
			jvmexit(1);
		}
	} else {
		throwNullPointerException();
	}
}
INS_END

INS_BEGIN(f_areturn) {
	// Object return value:
	jobject jref = operandStackPopObjectRef();

	pop_frame();

	// Push result:
	operandStackPushObjectRef(jref);
}
INS_END

INS_BEGIN(f_ireturn) {
	// Integer return value:
	jint jrenameint = operandStackPopJavaInt();

	pop_frame();

	// Push result:
	operandStackPushJavaInt(jrenameint);
}
INS_END

INS_BEGIN(f_lreturn) {
	// Long return value:
	jlong j = operandStackPopJavaLong();

	pop_frame();

	// Push result:
	operandStackPushJavaLong(j);
}
INS_END

INS_BEGIN(f_aload) {
	u1 index = getU1FromCode();
	operandStackPushVariableObjectRef(index);
}
INS_END

INS_BEGIN(f_aload_0) {
	operandStackPushVariableObjectRef(0);
}
INS_END

INS_BEGIN(f_aload_1) //
operandStackPushVariableObjectRef(1);
INS_END

INS_BEGIN(f_aload_2) //
operandStackPushVariableObjectRef(2);
INS_END

INS_BEGIN(f_aload_3) //
operandStackPushVariableObjectRef(3);
INS_END

INS_BEGIN(f_iload) //
u1 index = getU1FromCode();
operandStackPushVariableJavaInt(index);
INS_END

INS_BEGIN(f_iload_0) //
operandStackPushVariableJavaInt(0);
INS_END

INS_BEGIN(f_iload_1) //
operandStackPushVariableJavaInt(1);
INS_END

INS_BEGIN(f_iload_2) //
operandStackPushVariableJavaInt(2);
INS_END

INS_BEGIN(f_iload_3) //
operandStackPushVariableJavaInt(3);
INS_END

INS_BEGIN(f_lload) {
	u1 index = getU1FromCode();
	operandStackPushVariableJavaLong(index);
}
INS_END

INS_BEGIN(f_lload_0) {
	operandStackPushVariableJavaLong(0);
}
INS_END

INS_BEGIN(f_lload_1) {
	operandStackPushVariableJavaLong(1);
}
INS_END

INS_BEGIN(f_lload_2) {
	operandStackPushVariableJavaLong(2);
}
INS_END

INS_BEGIN(f_lload_3) {
	operandStackPushVariableJavaLong(3);
}
INS_END

INS_BEGIN(f_aconst_null) //
operandStackPushObjectRef((jobject) NULL);
INS_END

INS_BEGIN(f_iconst_m1) //
operandStackPushJavaInt(-1);
INS_END

INS_BEGIN(f_iconst_0) //
operandStackPushJavaInt(0);
INS_END

INS_BEGIN(f_iconst_1) //
operandStackPushJavaInt(1);
INS_END

INS_BEGIN(f_iconst_2) //
operandStackPushJavaInt(2);
INS_END

INS_BEGIN(f_iconst_3) //
operandStackPushJavaInt(3);
INS_END

INS_BEGIN(f_iconst_4) //
operandStackPushJavaInt(4);
INS_END

INS_BEGIN(f_iconst_5) //
operandStackPushJavaInt(5);
INS_END

INS_BEGIN(f_lconst_0) {
	operandStackPushJavaLong(0);
}
INS_END

INS_BEGIN(f_lconst_1) {
	operandStackPushJavaLong(1);
}
INS_END

INS_BEGIN(f_isub) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 - jrenameint1);
}
INS_END

INS_BEGIN(f_lsub) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	operandStackPushJavaLong(j2 - j1);
}
INS_END

INS_BEGIN(f_iand) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 & jrenameint1);
}
INS_END

INS_BEGIN(f_ior) //
jint jrenameint1 = operandStackPopJavaInt();
jint jrenameint2 = operandStackPopJavaInt();
operandStackPushJavaInt(jrenameint2 | jrenameint1);
INS_END

INS_BEGIN(f_ixor)
jint jrenameint1 = operandStackPopJavaInt();
jint jrenameint2 = operandStackPopJavaInt();
operandStackPushJavaInt(jrenameint2 ^ jrenameint1);
INS_END

INS_BEGIN(f_land) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	operandStackPushJavaLong(j2 & j1);
}
INS_END

INS_BEGIN(f_lor) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	operandStackPushJavaLong(j2 | j1);
}
INS_END

INS_BEGIN(f_lxor) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	operandStackPushJavaLong(j2 ^ j1);
}
INS_END

INS_BEGIN(f_iadd) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 + jrenameint1);
}
INS_END

INS_BEGIN(f_ladd) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	operandStackPushJavaLong(j2 + j1);
}
INS_END

INS_BEGIN(f_idiv) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	if (jrenameint1 == 0) {
		throwArithmeticException(divisionByZeroMessage);
	} else {
		operandStackPushJavaInt(jrenameint2 / jrenameint1);
	}
}
INS_END

INS_BEGIN(f_ldiv) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	if (j1 == 0) {
		throwArithmeticException(divisionByZeroMessage);
	} else {
		operandStackPushJavaLong(j2 / j1);
	}
}
INS_END

INS_BEGIN(f_ineg) {
	operandStackPushJavaInt(-operandStackPopJavaInt());
}
INS_END

INS_BEGIN(f_lneg) {
	operandStackPushJavaLong(-operandStackPopJavaLong());
}
INS_END

INS_BEGIN(f_irem) {
	jint v1 = operandStackPopJavaInt();
	jint v2 = operandStackPopJavaInt();
	if (v1 == 0) {
		throwArithmeticException(divisionByZeroMessage);
	} else {
		operandStackPushJavaInt(v2 % v1);
	}
}
INS_END

INS_BEGIN(f_lrem) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	if (j1 == 0) {
		throwArithmeticException(divisionByZeroMessage);
	} else {
		operandStackPushJavaLong(j2 % j1);
	}
}
INS_END

INS_BEGIN(f_imul) {
	jint jrenameint1 = operandStackPopJavaInt();
	jint jrenameint2 = operandStackPopJavaInt();
	operandStackPushJavaInt(jrenameint2 * jrenameint1);
}
INS_END

INS_BEGIN(f_lmul) {
	jlong j1 = operandStackPopJavaLong();
	jlong j2 = operandStackPopJavaLong();
	operandStackPushJavaLong(j2 * j1);
}
INS_END

INS_BEGIN(f_astore) //
u1 index = getU1FromCode();
operandStackPopVariableObjectRef(index);
INS_END

INS_BEGIN(f_astore_0) //
operandStackPopVariableObjectRef(0);
INS_END

INS_BEGIN(f_astore_1) //
operandStackPopVariableObjectRef(1);
INS_END

INS_BEGIN(f_astore_2) //
operandStackPopVariableObjectRef(2);
INS_END

INS_BEGIN(f_astore_3) //
operandStackPopVariableObjectRef(3);
INS_END

INS_BEGIN(f_istore) //
u1 index = getU1FromCode();
operandStackPopVariableJavaInt(index);
INS_END

INS_BEGIN(f_istore_0) //
operandStackPopVariableJavaInt(0);
INS_END

INS_BEGIN(f_istore_1) //
operandStackPopVariableJavaInt(1);
INS_END

INS_BEGIN(f_istore_2) //
operandStackPopVariableJavaInt(2);
INS_END

INS_BEGIN(f_istore_3) //
operandStackPopVariableJavaInt(3);
INS_END

INS_BEGIN(f_lstore) {
	u1 index = getU1FromCode();
	operandStackPopVariableJavaLong(index);
}
INS_END

INS_BEGIN(f_lstore_0) {
	operandStackPopVariableJavaLong(0);
}
INS_END

INS_BEGIN(f_lstore_1) {
	operandStackPopVariableJavaLong(1);
}
INS_END

INS_BEGIN(f_lstore_2) {
	operandStackPopVariableJavaLong(2);
}
INS_END

INS_BEGIN(f_lstore_3) {
	operandStackPopVariableJavaLong(3);
}
INS_END

INS_BEGIN(f_lcmp) {
	jlong val1 = operandStackPopJavaLong();
	jlong val2 = operandStackPopJavaLong();
	if (val2 > val1) {
		operandStackPushJavaInt(1);
	} else
	if (val2 < val1) {
		operandStackPushJavaInt(-1);
	} else {
		operandStackPushJavaInt(0);
	}
}
INS_END

/////////////////////////////////////////////////////////
// Instructions END
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Undefined instructions BEGIN
/////////////////////////////////////////////////////

INS_BEGIN(f_fconst_0) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fconst_1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fconst_2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dconst_0) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dconst_1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fload) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dload) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fload_0) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fload_1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fload_2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fload_3) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dload_0) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dload_1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dload_2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dload_3) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_faload) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_daload) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_saload) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fstore) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dstore) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fstore_0) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fstore_1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fstore_2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fstore_3) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dstore_0) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dstore_1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dstore_2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dstore_3) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fastore) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dastore) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_sastore) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dup_x2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dup2_x2) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_swap) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fadd) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dadd) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fsub) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dsub) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fmul) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dmul) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fdiv) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_ddiv) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_frem) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_drem) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fneg) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dneg) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_iushr) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_i2f) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_i2d) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_l2f) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_l2d) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_f2i) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_f2l) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_f2d) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_d2i) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_d2l) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_d2f) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_i2s) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fcmpl) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_fcmpg) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dcmpl) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dcmpg) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_jsr) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_ret) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_tableswitch) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_lookupswitch) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_freturn) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_dreturn) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_unused) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_wide) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_multianewarray) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_goto_w) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_jsr_w) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_breakpoint) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_thinj_undefined) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_impdep1) //
INS_UNDEFINED;
//
INS_END

INS_BEGIN(f_impdep2) //
INS_UNDEFINED;
//
INS_END

/////////////////////////////////////////////////////
// Undefined instructions END
/////////////////////////////////////////////////////


}

//From file:///tools/vmspec/Mnemonics.doc.html
//
//Contents | Prev | Next | Index	The JavaTM Virtual Machine Specification
//
//CHAPTER 9
//Opcode Mnemonics by Opcode
//
//This chapter gives the mapping from Java virtual machine instruction opcodes, including the reserved opcodes (§6.2), to the mnemonics for the instructions represented by those opcodes.
//
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
//
//1 For historical reasons, opcode value 186 is not used.
//
//Contents | Prev | Next | Index
//
//The JavaTM Virtual Machine Specification
//Copyright © 1999 Sun Microsystems, Inc. All rights reserved
//Please send any comments or corrections to jvm@java.sun.com
