/*
 * Java_java_io_PrintStream.c
 *
 *  Created on: Oct 28, 2011
 *      Author: hammer
 */

#include "jni.h"
#include "heap.h"
#include "console.h"


void JNICALL Java_java_io_PrintStream_outString(JNIEnv *env, jobject this, jstring s) {
	stackable* val = (stackable*) s;

	array* a = (array*) (val->operand.jref);
	int i;
	for (i = 0; i < a->header.length; i++) {
		consout("%c", a->data[i]);
	}
}

void JNICALL Java_java_io_PrintStream_outStringLf(JNIEnv *env, jobject this, jstring s) {
	Java_java_io_PrintStream_outString(env, this, s);
	consout("\n");
}

