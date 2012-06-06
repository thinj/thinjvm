/*
 * jni.h
 *
 *  Created on: Mar 18, 2011
 *      Author: hammer
 */

#ifndef JNI_H_
#define JNI_H_

#include "types.h"
#include "frame.h"
#include "heap.h"

#define JNIEXPORT extern
#define JNICALL

#define JNIEnv contextDef

/**
 * This macro assumes the OBJECT is a byte[] object, and sets S1PTR to an s1* pointing to the bytes in
 * the array and the LENGHT to the length of the array
 * \param OBJECT The jobject containing the byte[]
 * \param S1PTR The name of the s1* to be declared and assigned the address of byte[0]
 * \param LENGTH The name of the variable to be declared and assigned the number of bytes in the array
 */
#define OBJECT_TO_S1_ARRAY(S1PTR, LENGTH, OBJECT) \
	s1* S1PTR = (s1*) &((array*) OBJECT)->data[0]; \
	jint LENGTH = ((array*) OBJECT)->header.length

/**
 * This method calls a static method identified by 'referencedClassId' and 'linkId'. These two parameters
 * are used as lookup keys in the table 'allMethodsInAllClasses' where they shall match 'classId' and
 * 'linkId', respectively.
 *
 * \param referencedClassId The referenced class
 * \param linkId Identifies the method within the referenced class
 */
void call_static_method(u2 referencedClassId, u2 linkId);

/**
 * This method calls an instance method.
 *
 * \param referencedClassObject The referenced class. The classId of this instance is used when looking up
 * the target method in the method table 'allMethodsInAllClasses'.
 * \param linkId Identifies the method within the referenced class
 */
void call_instance_method(jobject referencedClassObject, u2 linkId);

#endif /* JNI_H_ */
