/*
 * Java_java_io_PrintStream.c
 *
 *  Created on: Oct 28, 2011
 *      Author: hammer
 */

#include "jni.h"
#include "heap.h"
#include "console.h"
#include "constantpool.h"
#include "vmids.h"

void JNICALL Java_java_io_PrintStream_outString(JNIEnv *env, jobject this, jstring s) {
	u2 linkId = LINK_ID_java_lang_String_value__C;
	jcharArray charArray = (jcharArray) GetObjectField(s, linkId);

	size_t length = GetArrayLength(charArray);
	size_t i;
	for (i = 0; i < length; i++) {
		consout("%c", GetCharArrayElement(charArray, i));
	}
}

void JNICALL Java_java_io_PrintStream_outStringLf(JNIEnv *env, jobject this, jstring s) {
	Java_java_io_PrintStream_outString(env, this, s);
	consout("\n");
}

