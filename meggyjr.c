/*
 * meggyjr.c
 *
 *  Created on: Jun 22, 2011
 *      Author: hammer
 */

#include "jni.h"
#include "console.h"

JNIEXPORT void JNICALL Java_thinj_meggyjr_MeggyJr_simpleSetup(JNIEnv *env, jclass cl) {
	consout("Setup\n");
}

JNIEXPORT void JNICALL Java_thinj_meggyjr_MeggyJr_delay(JNIEnv *env, jclass cl, jint millis) {
	usleep(1000 * millis);
}

JNIEXPORT void JNICALL Java_thinj_meggyjr_MeggyJr_setAuxLeds(JNIEnv *env, jclass cl, jint leds) {
	consout("leds = %02x\n", leds);
}
