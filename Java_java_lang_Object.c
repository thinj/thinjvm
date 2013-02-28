/*
 * Java_java_lang_Object.c
 *
 *  Created on: Mar 18, 2011
 *      Author: hammer
 */

#include <stdlib.h>

#include "console.h"
#include "heap.h"
#include "heaplist.h"

#include "jni.h"
#include "objectaccess.h"
#include "vmids.h"

jint Java_java_lang_Object_hashCode(void *context, jobject this) {
	return (jint) this;
}

jobject JNICALL Java_java_lang_Object_getClass(JNIEnv *env, jobject this) {
	u2 classId = oaGetClassIdFromObject(this);

	// Find Class instance that has classId 'classId':
	int i;
	jobject class = NULL;
	int len = GetArrayLength(javaLangClassArray);
	u2 aClassIdLinkId = LINK_ID_java_lang_Class_aClassId_I;
	for (i = 0; i < len && class == NULL; i++) {
		jobject classObject = GetObjectArrayElement(javaLangClassArray, i);

		jint classObjectDotaClassId = GetIntField(classObject, aClassIdLinkId);

		if (classObjectDotaClassId == (jint) classId) {
			class = classObject;
		}
	}

	if (class == NULL) {
		DEB(consout("NULL class!!!"));
	}
	return class;
}
