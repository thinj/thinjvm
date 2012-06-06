/*
 * heap.h
 *
 *  Created on: Oct 13, 2010
 *      Author: hammer
 */

#ifndef HEAP_H_
#define HEAP_H_

#include "types.h"
#include "config.h"
#include "operandstack.h"

#define HEAP_MAGIC 0x5F89

typedef struct __heapElement {
	//int magic;
	struct __heapElement* prev;
	struct __heapElement* next;
	// The size of the header and the bytes allocated besides the header:
	int size;
	// Mark and sweep variable for garbage collection:
	u1 mark;
	// The id of the class in this element:
	u2 classId;
	// Magic:
	u2 magic;
} heapElement;

/**
 * This struct contains stat about a heap list
 */
typedef struct __heapListStat {
	// The number of elements in list:
	int count;
	// The number of bytes in the list:
	size_t size;
} heapListStat;

/**
 * This struct contains stat about how many times GC has been run
 */
typedef struct __gcStat {
	// The number of mark and sweep GC's:
	int markAndSweepCount;
} gcStat;

/**
 * The header of an array on heap:
 */
typedef struct __arrayheader {
	// The number of elements in the array.
	jint length;

	// The type of the elements in the array - only valid for object arrays:
	u2 elementClassId;

} arrayheader;

/**
 * This struct contains an array on the heap
 */
typedef struct __array {
	// The header of the array (when splitting the array declaration in this manner its up to the compiler
	// to ensure proper alignment of field 'data' - e.g. the ARM architecture cannot accept pointers which
	// is not placed on a 4 - aligned address):
	arrayheader header;

	// The element data:
	u1 data[];
} array;

/**
 * This method returns the classId for an object
 * \param p A pointer to the java object for which the class id has to be returned
 * \returns The class id of p
 */
u2 getClassIdFromObject(jobject p);

/**
 * This method allocates a java object on the heap
 * \param size The size of the java object - in count of 'stackable's
 * \param classId The identification the class contained in the object
 */
jobject heapAllocObject(u2 size, u2 classId);

/**
 * This method allocates an array of simple types on the heap
 * \param count The number of elements
 * \param classId The id of array class
 * \param size The size of a single array element
 * \return The allocated object or null, if out of mem has been thrown
 */
jobject heapAllocPrimitiveTypeArray(jint count, size_t size, u2 classId);

/**
 * This method allocates an array on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \return The allocated object
 */
jobject heapAllocObjectArray(jint count, u2 elementClassId);

/**
 * This method initializes the heap
 */
void heapInit(void);

/**
 * This method executes simple garbage collection using mark and sweep algorithm.
 */
void markAndSweep(void);

/**
 * This method collects statistical information about heap usage
 * \param usedStat Pointer to the statistics for the list of used heap elements
 * \param usedStat Pointer to The statistics for the list of free heap elements
 * \param gc Pointer to the statistics about GCs
 */
void getHeapStat(heapListStat* usedStat, heapListStat* freeStat, gcStat* gc);

/**
 * This method sets the protected object. 'Protected' means that even if it found to be candidate
 * for garbage collection, it will not be garbage collected. It shal be set to NULL, when no object shall
 * be protected.
 * \param jref The object to protect, or null, if no object shall be protected.
 */
void heapProtect(jobject jref);

#define HEAP_ELEMENT_TO_OBJECT_REF(E) ((jobject) (((void *) E) + sizeof(heapElement)))
#define OBJECT_REF_TO_HEAP_ELEMENT(R) ((heapElement*) (((void*) R) - sizeof(heapElement)))

void validateStackables(stackable* memory, size_t length);

#endif /* HEAP_H_ */
