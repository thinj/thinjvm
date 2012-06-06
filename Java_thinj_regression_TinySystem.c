/*
 * Java_thinj_regression_TinySystem.c
 *
 *  Created on: Aug 19, 2011
 *      Author: hammer
 */

#include "jni.h"
#include "heap.h"
#include "console.h"

void JNICALL Java_thinj_regression_TinySystem_outInt(JNIEnv *env, jobject clazz, jint p0) {
	consout("%d", (int) p0);
}

void JNICALL Java_thinj_regression_TinySystem_outBoolean(JNIEnv *env, jobject clazz, BOOL p0) {
	consout(p0 ? "true" : "false");
}

void JNICALL Java_thinj_regression_TinySystem_outString(JNIEnv *env, jobject this, jstring s) {
	// TODO
	//    jboolean iscopy;
	//    const char *mfile = (*env)->GetStringUTFChars(env, s, &iscopy);

	stackable* val = (stackable*) s;

	array* a = (array*) (val->operand.jref);
	int i;
	for (i = 0; i < a->header.length; i++) {
		consout("%c", a->data[i]);
	}
	consout("\n");
}

JNIEXPORT void JNICALL Java_thinj_regression_TinySystem_outChar(JNIEnv *env, jobject clazz,
		jchar c) {
	consout("%c", c);
}
