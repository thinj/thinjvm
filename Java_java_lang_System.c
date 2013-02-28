/*
 * Java_java_lang_System.c
 *
 *  Created on: Jan 11, 2013
 *      Author: hammer
 */

#include <time.h>

#include "jni.h"
#include "architecture.h"
//#if ARCH == ARCH_ARM
//#include "blueboard.h"
//#endif


unsigned long long readNanoTimer();

JNIEXPORT jlong JNICALL Java_java_lang_System_nanoTime(JNIEnv *env, jclass cls) {
#if ARCH == ARCH_ARM
	return (jlong) readNanoTimer();
#elif ARCH == ARCH_NATIVE
	// see: http://linux.die.net/man/3/clock_gettime
	jlong j = 123456789012LL;
	struct timespec tp;
	// Link using librt:
	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
		j = 1000000000LL * (jlong) (tp.tv_sec) + (jlong) (tp.tv_nsec);
	}
	return j;
#endif
}
