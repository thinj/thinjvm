/*
 * Java_java_lang_Object.c
 *
 *  Created on: Mar 18, 2011
 *      Author: hammer
 */

#include <stdlib.h>

#include "console.h"
#include "heap.h"

#include "jni.h"

jint Java_java_lang_Object_hashCode(void *context, jobject this) {
	return (jint) this;
}

jobject JNICALL Java_java_lang_Object_getClass(JNIEnv *env, jobject this) {
	u2 classId = getClassIdFromObject(this);

	array* a = (array*) javaLangClassArray;

	jclass* oa = (jclass*) &a->data[0];

	// Find Class instance that has classId 'classId':
	int i;
	jobject class = NULL;
	for (i = 0; i < a->header.length && class == NULL; i++) {
		jobject classObject = oa[i];

		stackable* val = (stackable*) classObject;
		if (val->type != JAVAINT) {
			consout("JAVAINT expected; got %d\n", val->type);
			jvmexit(1);
		}
		jint classObjectDotaClassId = val->operand.jrenameint;
		if (classObjectDotaClassId == (jint) classId) {
			class = classObject;
		}
	}

	if (class == NULL) {
		DEB(consout("NULL class!!!"));
	}
	return class;
}
