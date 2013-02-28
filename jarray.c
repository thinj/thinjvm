/*
 * jarray.c
 *
 *  Created on: Sep 14, 2012
 *      Author: hammer
 */

#include "types.h"
#include "jarray.h"
#include "heap.h"
#include "heaplist.h"
#include "constantpool.h"
#include "jni.h"
#include "exceptions.h"

static size_t GetAlignedArrayHeaderSize() {
	return ToAlignedSize(sizeof(array_t));
}

/**
 * \return A void* the to the memory area where the array elements resides. The length field is not part of
 * the pointed-to area
 * \throws NullPointerException if array == null
 */
void* jaGetArrayPayLoad(jarray array) {
	align_t* at = (align_t*) GetObjectPayload(array);
	if (!ExceptionCheck()) {
		at += GetAlignedArrayHeaderSize();
	} else {
		at = NULL;
	}

	return (void*) at;
}

/**
 * This method validates the array pointer, the index value and returns a pointer to
 * the position in memory where index 0 is positioned (the first element in the array)
 *
 * \throws NullPointerException if array == NULL
 * \throws ArrayIndexOutOfBoundsException if ix < 0 or ix >= length of array
 * \return NULL, if an exception has been thrown; otherwise a valid pointer is returned.
 */
static void* GetPointerToArrayPosition(jarray array, jint ix) {
	void* p = jaGetArrayPayLoad(array);
	if (!ExceptionCheck()) {
		if (ix < 0 || ix >= GetArrayLength(array)) {
			throwArrayIndexOutOfBoundsException(ix, GetArrayLength(array));
			p = NULL;
		}
	}

	return p;
}

/**
 * This method returns the array control struct from the supplied array. Goes for both primitive array
 * and object arrays.
 * \param array Assumed to be a jarray
 * \return The array control struct for the array. It is the responsibility of the cakker to ensure
 * that the pointer indeed is an array
 */
static array_t* GetArrayHeader(jarray array) {
	return ((array_t*) GetObjectPayload(array));
}

void InitArray(jarray array, size_t length) { //, u2 elementClassId) {
	array_t* a = GetArrayHeader(array);
	a->length = length;
	//a->elementClassId = elementClassId;
}

size_t GetAlignedArraySize(size_t payloadSize) {
	return GetAlignedArrayHeaderSize() + ToAlignedSize(payloadSize);
}

// Common macro for returning a value in an array. All exception checks etc. are done,
// and the macro will 'return' (so don't place any code after use of this macro):
#define GET_ARRAY_ELEMENT(array, index, TYPE) \
		TYPE* p = (TYPE*) GetPointerToArrayPosition(array, index); \
		if (p != NULL) { \
			p += index; \
			return *p; \
		} else { \
			return (TYPE) 0; \
        }

// Common macro for setting a value in an array. All exception checks etc. are done:
#define SET_ARRAY_ELEMENT(array, index, value, TYPE) \
		HEAP_VALIDATE; \
		TYPE* p = (TYPE*) GetPointerToArrayPosition(array, index); \
		if (p != NULL) { \
			p += index; \
			*p = value; \
		} \
		HEAP_VALIDATE;

jobject GetObjectArrayElement(jarray array, size_t index) {
	GET_ARRAY_ELEMENT(array, index, jobject);
}

void SetObjectArrayElement(jarray array, size_t index, jobject value) {
	HEAP_VALIDATE;
//	consoutli("index = %d, array=0x%08x\n", index, getHeader(array));

	SET_ARRAY_ELEMENT(array, index, value, jobject);
}

size_t GetArrayLength(jarray array) {
	return GetArrayHeader(array)->length;
}

void SetByteArrayRegion(jbyteArray array, jsize start, jsize len, jbyte *buf) {
	HEAP_VALIDATE;

	jbyte* p = (jbyte*) GetPointerToArrayPosition(array, start);
	if (ExceptionCheck()) {
		return;
	}

	// Validate end also:
	GetPointerToArrayPosition(array, start + len - 1);

	if (ExceptionCheck()) {
		return;
	}

	if (p != NULL && buf != NULL) {
		p += start;
		while (len-- > 0) {
			*p = *buf;
			p++;
			buf++;
		}
	}HEAP_VALIDATE;
}

void GetByteArrayRegion(jbyteArray array, jsize start, jsize len, jbyte *buf) {
	HEAP_VALIDATE;

	jbyte* p = (jbyte*) GetPointerToArrayPosition(array, start);
	if (ExceptionCheck()) {
		return;
	}

	// Validate end also:
	GetPointerToArrayPosition(array, start + len - 1);

	if (ExceptionCheck()) {
		return;
	}

	if (p != NULL && buf != NULL) {
		p += start;
		while (len-- > 0) {
			*buf = *p;
			p++;
			buf++;
		}
	}
	HEAP_VALIDATE;
}

jarray NewObjectArray(jint count, u2 elementClassId, jobject init) {
	HEAP_VALIDATE;
	u2 arrayClassId = getArrayClassIdForElementClassId(elementClassId);
	size_t size = sizeof(jobject);
	// The payload size:
	u2 payloadSize = count * size;

	size_t alignedSizeInBytes = GetAlignedArraySize(payloadSize) * sizeof(align_t);

	jarray array = heapAllocObjectByByteSize(alignedSizeInBytes, arrayClassId);

	if (array == NULL) {
		throwOutOfMemoryError();
	} else {
		InitArray(array, count); //, elementClassId);
		jint i;
		for (i = 0; i < count; i++) {
			SetObjectArrayElement(array, i, init);
		}
	}

	HEAP_VALIDATE;
	return array;
}

jboolean GetBooleanArrayElement(jarray array, size_t index) {
	GET_ARRAY_ELEMENT(array, index, jboolean);
}

jbyte GetByteArrayElement(jarray array, size_t index) {
	GET_ARRAY_ELEMENT(array, index, jbyte);
}

jchar GetCharArrayElement(jarray array, size_t index) {
	GET_ARRAY_ELEMENT(array, index, jchar);
}

jint GetIntArrayElement(jarray array, size_t index) {
	GET_ARRAY_ELEMENT(array, index, jint);
}

jint GetLongArrayElement(jarray array, size_t index) {
	GET_ARRAY_ELEMENT(array, index, jlong);
}

static jarray NewPrimitiveArray(CLASS_TYPE classType, size_t len, size_t elementSize) {
	HEAP_VALIDATE;
	u2 arrayClassId = getClassIdForClassType(classType);

	u2 payloadSize = len * elementSize;

	size_t alignedSizeInBytes = GetAlignedArraySize(payloadSize) * sizeof(align_t);

	jarray array = heapAllocObjectByByteSize(alignedSizeInBytes, arrayClassId);

	if (array != NULL) {
		InitArray(array, len);//, 0);
	}
	//else: out of mem has been thrown
	HEAP_VALIDATE;

	return array;
}

jcharArray NewCharArray(size_t len) {
	return NewPrimitiveArray(CT_CHAR_ARRAY, len, sizeof(jchar));
}

jbooleanArray NewBooleanArray(size_t len) {
	return NewPrimitiveArray(CT_BOOLEAN_ARRAY, len, sizeof(jboolean));
}

jbyteArray NewByteArray(size_t len) {
	return NewPrimitiveArray(CT_BYTE_ARRAY, len, sizeof(jbyte));
}

jintArray NewIntArray(size_t len) {
	return NewPrimitiveArray(CT_INT_ARRAY, len, sizeof(jint));
}

jlongArray NewLongArray(size_t len) {
	return NewPrimitiveArray(CT_LONG_ARRAY, len, sizeof(jlong));
}

void SetBooleanArrayElement(jarray array, size_t index, BOOL value) {
	SET_ARRAY_ELEMENT(array, index, value, jboolean);
}

void SetByteArrayElement(jarray array, size_t index, jbyte value) {
	SET_ARRAY_ELEMENT(array, index, value, jbyte);
}

void SetIntArrayElement(jarray array, size_t index, jint value) {
	SET_ARRAY_ELEMENT(array, index, value, jint);
}

void SetLongArrayElement(jarray array, size_t index, jlong value) {
	SET_ARRAY_ELEMENT(array, index, value, jlong);
}

void SetCharArrayElement(jarray array, size_t index, jchar value) {
	SET_ARRAY_ELEMENT(array, index, value, jchar);
}
