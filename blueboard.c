/*
 * blueboard.c
 *
 *  Created on: Jun 22, 2011
 *      Author: hammer
 */

#include "jni.h"
#include "heap.h"
#include "console.h"

JNIEXPORT void JNICALL Java_thinj_blueboard_Blueboard_init(JNIEnv *env, jclass cl) {
	consout("init\n");
}
JNIEXPORT void JNICALL Java_thinj_blueboard_Blueboard_usbBlink(JNIEnv *env, jclass cl) {
	consout("usb blink\n");
	while (1) {
		usleep(1000000);
	}
}

void JNICALL Java_thinj_blueboard_Blueboard_outString(JNIEnv *env, jclass this, jstring s) {
	stackable* val = (stackable*) s;
	array* a = (array*) (val->operand.jref);
	int i;
	for (i = 0; i < a->header.length; i++) {
		char c = a->data[i];
		consout("%c", c);
	}
	consout("\n");
}

void JNICALL Java_thinj_blueboard_Blueboard_outInt(JNIEnv *env, jobject clazz, jint p0) {
	consout("%d\n", (int) p0);
}

void JNICALL Java_thinj_blueboard_Blueboard_delay(JNIEnv *context, jclass jc, jint millis) {
	usleep(millis * 1000);
}

void JNICALL Java_thinj_blueboard_Blueboard_usbLed(JNIEnv *env, jclass jc, BOOL on) {
	consout("led: %s\n", on ? "ON" : "OFF");
}

void JNICALL Java_thinj_blueboard_Blueboard_buzzer(JNIEnv *env, jclass jc, BOOL on) {
	consout("buzzer: %s\n", on ? "ON" : "OFF");

}
void JNICALL Java_thinj_gameduino_Gameduino_writeByte(JNIEnv *env, jclass jc, jint address,
		jint val) {
	// Fake
}

jint JNICALL Java_thinj_gameduino_Gameduino_readByte(JNIEnv *env, jclass jc, jint address) {
	// FAKE
	return -1;
}

void JNICALL Java_thinj_gameduino_Gameduino_writeShort(JNIEnv *env, jclass jc, jint addr, jint val) {
	// Fake
}

void JNICALL Java_thinj_gameduino_Gameduino_readBytes(JNIEnv *env, jclass jc, jint address,
		jobject byteArray) {

	OBJECT_TO_S1_ARRAY(data, len, byteArray);
	int n;
	for (n = 0; n < len; n++) {
		consout("s1a: %d\n", (int) data[n]);
	}


	consout("array length: %d\n", len);
	int i;
	for (i = 0; i < len; i++) {
		int val = data[i];
		consout("%d ", val);
	}

	for (i = 0; i < len; i++) {
		data[i] = i + 20;
	}
	consout("\n");

}

