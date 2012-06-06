/*
 * Java_java_lang_Class.c
 *
 *  Created on: Aug 19, 2011
 *      Author: hammer
 */

#include "jni.h"

jobject JNICALL Java_java_lang_Class_getAllClasses(JNIEnv *env, jclass cl) {
	// The array of java.lang.Class:
	return javaLangClassArray;

}
