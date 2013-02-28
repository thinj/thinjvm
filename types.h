/*
 * types.h
 *
 *  Created on: Aug 21, 2010
 *      Author: hammer
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>

/**
 * This type defines the alignment of heap elements and defines the granularity of memory chunks
 */
typedef int align_t;

typedef enum {
	FALSE = 0, TRUE = 1
} BOOL;

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef signed char s1;
typedef signed short s2;
typedef signed int s4;
typedef long long signed int s8;
typedef long long unsigned int u8;

typedef u2 codeIndex;

/**
 * For native type defs, see http://download.oracle.com/javase/1.5.0/docs/guide/jni/spec/types.html
 */

typedef s2 jshort;
typedef s1 jbyte;
typedef s1 jboolean;
typedef s1 jchar;
typedef s4 jint;
typedef s8 jlong;
typedef u8 ujlong; // well, has no java equivalent

typedef float jfloat;
typedef double jdouble;
typedef void* jobject;
typedef jobject jarray;
typedef jarray jobjectArray;
typedef jarray jbyteArray;
typedef jarray jbooleanArray;
typedef jarray jcharArray;
typedef jarray jintArray;
typedef jarray jlongArray;
typedef jobject jclass;
typedef jobject jstring;
typedef jint jsize;

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

/**
 * Enumeration used for constants e.g. in 'LDC' instruction:
 */
typedef enum {
	CONSTANT_INTEGER, CONSTANT_LONG, CONSTANT_FLOAT, CONSTANT_STRING, CONSTANT_CLASS
} CONSTANT_TYPE;

/**
 * Enumeration used for type of class
 */
typedef enum {
	CT_CLASS, CT_INTERFACE,
	// For easy casting use the same values as for ARRAY_TYPE:
	CT_OBJECT_ARRAY = 3,
	CT_BOOLEAN_ARRAY = 4,
	CT_CHAR_ARRAY = 5,
	CT_FLOAT_ARRAY = 6,
	CT_DOUBLE_ARRAY = 7,
	CT_BYTE_ARRAY = 8,
	CT_SHORT_ARRAY = 9,
	CT_INT_ARRAY = 10,
	CT_LONG_ARRAY = 11
} CLASS_TYPE;

typedef union __constantValue {
	jint jrenameint;
	jlong jlong;
	jchar* string;
	u2 classId; // For CONSTANT_CLASS
	// float
	// String
} constantValue;

/**
 * Enumeration used for array element types
 */
typedef enum {
	T_REFERENCE = 3, // This is NOT part of JVM spec - but only a convenience for Object[]
	T_BOOLEAN = 4,
	T_CHAR = 5,
	T_FLOAT = 6,
	T_DOUBLE = 7,
	T_BYTE = 8,
	T_SHORT = 9,
	T_INT = 10,
	T_LONG = 11,
} ARRAY_TYPE;

// Constant used in e.g. 'LDC' instruction:
typedef struct __constantDef {
	CONSTANT_TYPE type;
	constantValue value;
} constantDef;

/**
 * Enumeration of the different kind of heap elements:
 */
typedef enum {
	// The element is not yet refered from any refered object, so don't GC it. However, it shall be
	// regarded as an object in state HT_USED when validating heap:
	HT_PROTECTED = 5,
	// The element is free and can be allocated:
	HT_FREE = 9,
	// The element is in used and cannot be allocated:
	HT_USED = 0xd
} hdrtype_t;

#define MARK_BIT_SIZE 4
typedef union __header {
	struct {
		// The type of the header:
		hdrtype_t type :4;

		// Current mark-and-sweep value:
		unsigned int mark :MARK_BIT_SIZE;

		// The id of the class in this element:
		u2 classId;

		// Next element:
		union __header* next;

		// The entire size used by this element including header it self (in #align_t, not bytes):
		size_t size;
	} e;
	align_t alignment;
} header_t;

/**
 * This method converts a size in bytes into a size in count of alignt_t
 */
size_t ToAlignedSize(size_t size_in_bytes);

/**
 * The number of align_t to allocate for the header:
 */
extern size_t HEAP_HEADER_SIZE;

/**
 * This function returns a pointer to the instance payload of the object
 * \param obj The object for which the payload is returned
 * \return The instance payload of the object
 */
void* GetObjectPayload(jobject obj);

#endif /* TYPES_H_ */
