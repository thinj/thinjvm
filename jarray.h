/*
 * jarray.h
 *
 *  Created on: Sep 14, 2012
 *      Author: hammer
 */

#ifndef JARRAY_H_
#define JARRAY_H_

#include "types.h"

/**
 * This struct contains an array on the heap
 */
typedef struct __array {
	// The number of elements in the array.
	jint length;
} array_t;

/**
 * This function initializes an Object array
 * \param array The Object array to initialize
 * \param length The number of elements in the array
 * \param elementClassId The classId of each element in the array
 */
void InitArray(jarray array, size_t length);
//void InitObjectArray(jarray array, size_t length, u2 elementClassId);

/**
 * This function returns the aligned size (in counts of align_t) of the
 * entire Object Array including control structure and payload
 * \param payloadSize The size of the payload (in bytes)
 * \return The aligned size (in counts of align_t) of the entire Object
 *  Array including control structure and payload
 */
size_t GetAlignedArraySize(size_t payloadSize);

/**
 * \return A void* the to the memory area where the array elements resides. The length field is not part of
 * the pointed-to area
 * \throws NullPointerException if array == null
 */
void* jaGetArrayPayLoad(jarray array);

#endif /* JARRAY_H_ */
