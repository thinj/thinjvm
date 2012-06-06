/*
 * types.h
 *
 *  Created on: Aug 21, 2010
 *      Author: hammer
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>

typedef enum {
	FALSE = 0, TRUE = 1
} BOOL;

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef signed char s1;
typedef signed short s2;
typedef signed int s4;
typedef long signed int u8;

typedef u2 codeIndex;

/**
 * For native type defs, see http://download.oracle.com/javase/1.5.0/docs/guide/jni/spec/types.html
 */

typedef s2 jshort;
typedef s1 javabyte;
typedef s1 jchar;
typedef s4 jint;
typedef float jfloat;
typedef double jdouble;
typedef u1* jobject;
typedef u1* jclass; // jclass extends jobject
typedef u1* jstring; // jstring extends jobject


/**
 * Enumeration used for constants e.g. in 'LDC' instruction:
 */
typedef enum {
	CONSTANT_INTEGER, CONSTANT_FLOAT, CONSTANT_STRING, CONSTANT_CLASS
} CONSTANT_TYPE;

/**
 * Enumeration used for type of class
 */
typedef enum {
	CT_CLASS,
	CT_INTERFACE,
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
	char* string;
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

#endif /* TYPES_H_ */
