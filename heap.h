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

//#define USED_HEAP_MAGIC 0x5F89
//#define FREE_HEAP_MAGIC 0xf27b

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
 * This method allocates a java object on the heap. The payload is cleared.
 * \param size The size of the java object - in count of sizeof(stackable)
 * \param classId The identification the class contained in the object
 * \return The allocated object. NB! Use appropriate methods for access; do not use the
 * returned pointer directly!
 */
jobject heapAllocObjectByStackableSize(u2 size, u2 classId);

/**
 * This method allocates a java object on the heap. The payload is cleared.
 * \param size The size of the java object - in count of bytes
 * \param classId The identification the class contained in the object
 * \return The allocated object. NB! Use appropriate methods for access; do not use the
 * returned pointer directly!
 */
jobject heapAllocObjectByByteSize(u2 size, u2 classId);


/**
 * This method initializes the heap
 * \param heap A pointer to the memory area where the heap will be placed
 * \param heapSize The size of the heap area (in count of align_t)
 */
void heapInit(align_t* heap, size_t heapSize);

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
 * \param protected true, if the object shall be protected, false otherwise
 */
void heapProtect(jobject jref, BOOL protected);

void validateStackables(stackable* memory, size_t length);

#endif /* HEAP_H_ */
