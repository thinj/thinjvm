/*
 * management.c
 *
 *  Created on: Nov 16, 2012
 *      Author: hammer
 */

#include "heaplist.h"
#include "types.h"
#include "jni.h"

jint JNICALL Java_thinj_VirtualMachine_getMaxHeap(JNIEnv *env, jclass jc) {
   return (jint) heap_max();
}

jint JNICALL Java_thinj_VirtualMachine_getHeapUsage(JNIEnv *env, jclass jc) {
    return (jint) heap_used();
}
