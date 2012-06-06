/*
 * Java_thinj_regression_ReverseNativeTest.c
 *
 *  Created on: Feb 17, 2012
 *      Author: hammer
 */

#include "jni.h"
#include "davsdu.h"
#include "console.h"

#ifdef CLASS_ID_thinj_regression_ReverseNativeStaticTest
void JNICALL Java_thinj_regression_ReverseNativeStaticTest_foo(JNIEnv *env, jclass cl) {
	consout("calling JVM bar...\n");

	call_static_method(CLASS_ID_thinj_regression_ReverseNativeStaticTest,
			LINK_ID_thinj_regression_ReverseNativeStaticTest_bar___V);

	consout("return to foo from bar\n");
}
#endif

#ifdef LINK_ID_thinj_regression_nativetest_ReverseNativeInstanceTest_bar___V
// 2: Java_thinj_regression_nativetest_ReverseNativeInstanceTest_foo
void JNICALL Java_thinj_regression_nativetest_ReverseNativeInstanceTest_foo(JNIEnv *env, jobject this) {
	consout("foo called\n");
	// Push 'this':
	operandStackPushObjectRef(this);
	call_instance_method(this,
			LINK_ID_thinj_regression_nativetest_ReverseNativeInstanceTest_bar___V);
}
#endif
