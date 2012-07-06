/*
 * heap.c
 *
 *  Created on: Oct 13, 2010
 *      Author: hammer
 */

#include <stdlib.h>
#include <string.h>
#include "constantpool.h"
#include "instructions.h"
#include "console.h"
#include "heap.h"
#include "frame.h"
#include "operandstack.h"
#include "debugger.h"
#include "trace.h"

static void dumpUsedList(void);
static void dumpFreeList(void);
void dumpHeap(void);
static u2 requestedSize = 0;


// internal forward declaration:
static void markObjects(u1 mark, stackable* memory, size_t length);

// The list of free elements on the heap:
static heapElement* freeList;

// The list of allocated elements on the heap:
static heapElement* usedList;

// The heap:
u1 * heap;
// The size of the heap:
size_t HEAP_SIZE;

// forwards:
/**
 * This method frees a heap element and joins it with any other adjacent elements
 * \param p The element to free
 */
static void heapFree(heapElement* p);

#if 0
/**
 * VALIDATE_HEAP_LISTS will validate the heap whenever executed. Undefine it
 * for speed optimization only - if speed isn't necessary let it stay, since
 * it will provide runtime checks of heap sanity.
 */
#define VALIDATE_HEAP_LISTS CALL(validateHeapLists(__LINE__))

/**
 * This method validates the indicated heap list
 */
static size_t validateList(int lineNumber, heapElement* list, char * text, BOOL checkSequence) {
	BEGIN;
	heapElement* p = list;
	int i = 0;
	size_t memUsed = 0;

	while (p != NULL && i < HEAP_SIZE / sizeof(heapElement)) {
		memUsed += p->size;
		i++;
		if (checkSequence) {
			if (p->next != NULL && p > p->next) {
				consout("'%s' is broken - Out of sequence - line %d\n", text, lineNumber);
				jvmexit(1);
			}
		} else {
			// Check magic:
			if (p->magic != HEAP_MAGIC) {
				DUMP_STACKTRACE("Validate Magic");
				jvmexit(1);
			}
		}
		p = p->next;
	}

	if (i >= HEAP_SIZE / sizeof(heapElement)) {
		consout("'%s' is broken - line %d\n", text, lineNumber);
		jvmexit(1);
	}
	END;
	return memUsed;
}

/**
 * This method validates the usedList and the freeList
 */
static void validateHeapLists(int lineNumber) {
	BEGIN;
	CALL(validateList(lineNumber, freeList, "freeList", TRUE));
	CALL(validateList(lineNumber, usedList, "usedList", FALSE));
	END;
}
#else
#define VALIDATE_HEAP_LISTS
#endif

// The number of mark and sweep GCs:
int markAndSweepCount = 0;

void heapInit(void* _heap, size_t heapSize) {
	BEGIN;
	usedList = NULL;

	heap = _heap;
	HEAP_SIZE = heapSize;
	freeList = (heapElement*) &heap[0];
	freeList->prev = NULL;
	freeList->next = NULL;
	freeList->size = HEAP_SIZE;
	VALIDATE_HEAP_LISTS;
	END;
}

// This macro inserts an item P in the start of a list LIST. The pointers in the entry P are modified.
#define PREPEND(LIST, P) \
	do {\
        P->next = LIST; \
        P->prev = NULL; \
        if (LIST != NULL) { \
        	LIST->prev = P; \
        } \
        LIST = P; \
	} while (FALSE)

// This macro removes an entry P from a list LIST. The entry it self is not modified. The list might
// be modified
#define REMOVE_FROM_LIST(LIST, P) \
	do {\
		heapElement* hp = LIST;\
		while (hp != NULL && hp != P) {\
			hp = hp->next;\
		}\
		if (hp == NULL) { \
			DEB(consout("Not in list\n")); \
			jvmexit(1);\
        }\
		if (P->prev == NULL) { \
			if (P != LIST) {\
				DEB(consout("P is broken\n"));\
				jvmexit(1);\
			}\
			/* It's the first element */\
			LIST = P->next;\
			if (LIST != NULL) {\
				LIST->prev = NULL;\
			}\
		} else if (P->next == NULL) {\
			/* It's the last, but not the first element (P->prev!= NULL): */\
			P->prev->next = NULL;\
		} else {\
			/* It's not the last and not the first element: */\
			P->prev->next = P->next;\
			P->next->prev = P->prev;\
		}\
		P->next = P->prev = NULL;\
	} while (FALSE)\
	//
u2 getClassIdFromObject(jobject p) {
	return OBJECT_REF_TO_HEAP_ELEMENT(p)->classId;
}

/**
 * This method allocates a java object on the heap
 * \param size The size of the java object - and the size in bytes, not in count of 'stackable's
 * \param classId The identification the class contained in the object
 * \return The allocated object or null, if out of mem. If out of mem, an exception shall be thrown
 * by the caller
 */
static jobject _heapAlloc(u2 size, u2 classId) {
	BEGIN;
	requestedSize = size;
	heapElement* p = freeList;
	heapElement* candidate = NULL;
	VALIDATE_HEAP_LISTS;

	// Alignment:
	u2 totalSize = (sizeof(heapElement) + size + 3) & (0xffff - 3);
	// u2 totalSize = (sizeof(heapElement) + size);
	// Find the smallest element which is able to contain the requested size:
	while (p != NULL) {
		// if p->size == totalSize we have a fine candidate:
		if (p->size == totalSize) {
			candidate = p;
			break;
		} else if (p->size > totalSize) {
			if (candidate != NULL) {
				if (p->size < candidate->size) {
					// p is better fit than candidate:
					candidate = p;
				}
				// else: 'candidate' is still the best candidate
			} else {
				candidate = p;
			}
		}
		// else: p->size < totalSize; continue search
		p = p->next;
	}VALIDATE_HEAP_LISTS;

	jobject jref;

	if (candidate != NULL) {
		// Candidate is now unlinked from freeList
		// Only allocate what's needed, however, don't split if the superfluous part cannot hold a header:
		if (candidate->size > totalSize + sizeof(heapElement)) {
			VALIDATE_HEAP_LISTS;
			// split:
			int remainingSize = candidate->size - totalSize;
			heapElement* remaining = (heapElement*) (((void*) candidate) + totalSize);
			remaining->size = remainingSize;
			// The remaining item shall be inserted into the freeList at the right position:
			// Re-use the position of candidate in freeList:
			remaining->next = candidate->next;
			remaining->prev = candidate->prev;
			if (remaining->next != NULL) {
				remaining->next->prev = remaining;
			}
			// else: Remaining is at end of list
			if (remaining->prev != NULL) {
				remaining->prev->next = remaining;
			} else {
				// Remaining is beginning of list:
				freeList = remaining;
			}

			candidate->size = totalSize;
			VALIDATE_HEAP_LISTS
			;
		} else {
			// Don't split; remove candidate from freeList:
			VALIDATE_HEAP_LISTS;
			REMOVE_FROM_LIST(freeList, candidate);VALIDATE_HEAP_LISTS
			;
		}

		// Add candidate to usedList:
		VALIDATE_HEAP_LISTS;
		if (candidate == usedList) {
			DEB(consout("candidate == usedList\n"));
		}
		PREPEND(usedList, candidate);
		candidate->magic = HEAP_MAGIC;
		VALIDATE_HEAP_LISTS;

		candidate->classId = classId;
		candidate->mark = 0;
		VALIDATE_HEAP_LISTS;
		jref = HEAP_ELEMENT_TO_OBJECT_REF(candidate);
	} else {
		jref = NULL;
	}

	if (jref != NULL) {
		memset((void*) jref, 0, size);
	}

	VALIDATE_HEAP_LISTS;

	END;
	return jref;
}

/**
 * This method allocates a java object on the heap. If out of mem at first try, the garbage collection
 * is run. Then another allocation attempt is made - and if out of mem, the 'out of mem' exception is thrown.
 * \param size The size of the java object - and the size in bytes, not in count of 'stackable's
 * \param classId The identification the class contained in the object
 * \return null if out of mem has been thrown
 */
static jobject heapAlloc(u2 size, u2 classId) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	//validateStackables(stack, context.operandStackPointer);

	CALL(jobject jref = _heapAlloc(size, classId));
	VALIDATE_HEAP_LISTS;
	if (jref == NULL) {
		CALL(markAndSweep());
		CALL(jref = _heapAlloc(size, classId))
		;
	}
	if (jref == NULL) {
		CALL(throwOutOfMemoryError())
		;
	}

	VALIDATE_HEAP_LISTS;

	END;
	return jref;
}

jobject heapAllocObject(u2 size, u2 classId) {
	jobject jref;
	BEGIN;
	//validateStackables(stack, context.operandStackPointer);
	jref = heapAlloc(size * sizeof(stackable), classId);

	END;
	return jref;
}

/**
 * This method will join two adjacent items in the linked list, if possible
 * \param p1 The n'th element from the list to join with the subsequent element (if any, and if
 * subsequent element is placed at an adjacent address in memory)
 */
void tryConcat(heapElement* p1) {
	BEGIN;
	heapElement* p2 = p1->next;
	if (p2 != NULL) {
		if (((void*) p1) + p1->size == (void*) p2) {
			// Yes, they are neighbours - use p1 and dispose p2:
			p1->next = p2->next;
			p1->size += p2->size;
			if (p1->next != NULL) {
				p1->next->prev = p1;
			}
		}
	}
	END;
}

/**
 * This method frees a heap element and joins it with any other adjacent elements
 */
static void heapFree(heapElement* p) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	if (p->classId == javaLangClassClassIndex) {
		DEB(consout("Freeing java.lang.Class ?!\n"));
		jvmexit(1);
	}

	// Remove from usedList:
	REMOVE_FROM_LIST(usedList, p);VALIDATE_HEAP_LISTS;

	// Merge into freeList:
	// Sorting relation: Let address X be less than address Y <=> X < Y is true
	// The freeList is sorted having the 'smallest' item first and then 'increasing' items.
	// Find the 'smallest' element that is 'larger' than p:
	heapElement* largerElement = freeList;
	heapElement* lastElement = largerElement;
	while (largerElement != NULL && largerElement < p) {
		lastElement = largerElement;
		largerElement = largerElement->next;
	}

	// Now e > p or e == NULL or lastElement = NULL
	if (lastElement == NULL) {
		// The freeList was empty
		freeList = p;
		p->prev = NULL;
		p->next = NULL;
		VALIDATE_HEAP_LISTS
		;
	} else {
		if (largerElement == NULL) {
			// p is larger than all elements - simply append it to the end of list:
			// INSERT p AFTER lastElement
			lastElement->next = p;
			p->prev = lastElement;
			p->next = NULL;
			VALIDATE_HEAP_LISTS;
			CALL(tryConcat(lastElement));
			VALIDATE_HEAP_LISTS
			;
		} else {
			// p < largerElement
			// INSERT p BEFORE largerElement
			p->next = largerElement;
			p->prev = largerElement->prev;
			if (largerElement->prev != NULL) {
				largerElement->prev->next = p;
			} else {
				freeList = p;
			}
			largerElement->prev = p;
			VALIDATE_HEAP_LISTS;

			// Can we join p and largerElement:
			CALL(tryConcat(p));
			VALIDATE_HEAP_LISTS;
			if (p->prev != NULL) {
				// Can we join p and its predecessor:
				tryConcat(p->prev);
				VALIDATE_HEAP_LISTS
				;
			} else {
				// No previous => p must be equal to the free list:
				freeList = p;
				VALIDATE_HEAP_LISTS
				;
			}
		}
	}
	END;
	VALIDATE_HEAP_LISTS
	;
}

jobject heapAllocPrimitiveTypeArray(jint count, size_t size, u2 classId) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	jobject ref;
	u2 payloadSize = count * size;

	CALL(ref = heapAlloc(payloadSize + sizeof(array), classId));
	if (ref != NULL) {
		array* a = ((array*) ref);
		a->header.length = count;
		a->header.elementClassId = 0; // Don't care

		// Clear all bytes in array:
		memset(&a->data[0], 0, payloadSize);
		VALIDATE_HEAP_LISTS;
	}
	// else: out of mem has been thrown
	END;

	return ref;
}

jobject heapAllocObjectArray(jint count, u2 elementClassId) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	size_t size = sizeof(jobject);
	// The payload size:
	u2 payloadSize = count * size;

	u2 classId = getArrayClassIdForElementClassId(elementClassId);
	CALL(jobject ref = heapAlloc(payloadSize + sizeof(array), classId));

	if (ref == NULL) {
		consout("out of mem %d\n", __LINE__);
	}
	array* a = ((array*) ref);
	a->header.length = count;
	a->header.elementClassId = elementClassId;

	// Clear all bytes in array:
	memset(&a->data[0], 0, payloadSize);
	VALIDATE_HEAP_LISTS;

	END;
	return ref;
}

/**
 * This method marks all objects referenced from the memory
 * \param mark The mark to mark with
 * \param memory The array of 'stackable' wherein the object refs shall be found
 * \param length The length of the 'stackable array.
 */
static void markObject(u1 mark, jobject jref) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	if (jref != NULL) {
		heapElement* hep = OBJECT_REF_TO_HEAP_ELEMENT(jref);

		if (hep->magic != HEAP_MAGIC) {
			consout("object: %08x %d mark = %d\n", hep, sizeof(heapElement), mark);
			consout("classId: %d\n", (int) hep->classId);
			consout("size: %d\n", (int) hep->size);
			consout("mark: %d\n", (int) hep->mark);
			consout("magic: %04x\n", hep->magic);

			dumpHeap();
			DUMP_STACKTRACE("xxxMagic");
			jvmexit(1);
		}

		// Mark the object it self:
		heapElement* e = OBJECT_REF_TO_HEAP_ELEMENT(jref);
		// Don't mark an object that's already marked - otherwise circle references will
		// cause endless recursion:
		if (e->mark != mark) {
			e->mark = mark;
			// Mark the referenced objects:
			u2 classId = e->classId;
			if (isObjectArray(classId)) {
				// TODO Consider: Shall an object array consist of stackables
				// TODO => markObjects method will work as well on any array?!
				// Mark this array:
				array* a = (array*) jref;
				jobject* refa = (jobject*) &(a->data[0]);
				int j;
				for (j = 0; j < a->header.length; j++) {
					// Note: Recursion:
					markObject(mark, refa[j]);
				}
			} else if (isPrimitiveValueArray(classId)) {
				// No further marking to do
			} else {
				// It's a 'normal' object
				// Mark all objects that this object contains references to:
				// Note: Recursion:
				u2 count;
				getClassSize(e->classId, &count);
				CALL(markObjects(mark, (stackable*) jref, count))
				;
			}
		}
	}
	// else: No marking of null pointers ;-)
	VALIDATE_HEAP_LISTS;
	END;
}
/**
 * This method marks all objects referenced from the memory
 * \param mark The mark to mark with
 * \param memory The array of 'stackable' wherein the object refs shall be found
 * \param length The length of the 'stackable array.
 */
static void markObjects(u1 mark, stackable* memory, size_t length) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	int i;

	for (i = 0; i < length; i++) {
		if (memory[i].type == OBJECTREF) {
			VALIDATE_HEAP_LISTS;
			// Mark the object it self:
			CALL(markObject(mark, memory[i].operand.jref));
			VALIDATE_HEAP_LISTS
			;
		}
	}VALIDATE_HEAP_LISTS;
	END;
}

void _validateStackables(stackable* memory, size_t length) {
	BEGIN;

	int i;

	for (i = 0; i < length; i++) {
		if (memory[i].type == OBJECTREF) {
			jobject jref = memory[i].operand.jref;
			if (jref != NULL) {
				heapElement* hep = OBJECT_REF_TO_HEAP_ELEMENT(jref);

				if (hep->magic != HEAP_MAGIC) {
					consout("object: %08x %d\n", hep, sizeof(heapElement));
					consout("classId: %d\n", (int) hep->classId);
					consout("size: %d\n", (int) hep->size);
					consout("mark: %d\n", (int) hep->mark);
					consout("magic: %04x\n", hep->magic);

					dumpHeap();
					dumpStackTrace();
					consout("pc: %04x\n", context.programCounter);
					DUMP_STACKTRACE("yyyMagic");
					jvmexit(1);
				}
			}
		}
	}
	END;
}

/**
 * This method marks all objects referenced from the static memory
 * \param mark The mark to mark with
 */
static void markStaticObjects(u1 mark) {
	BEGIN;
	VALIDATE_HEAP_LISTS;
	CALL(markObjects(mark, staticMemory, (size_t) staticMemorySize));
	VALIDATE_HEAP_LISTS;
	END;
}

/**
 * This method marks all objects referenced from the stack
 * \param mark The mark to mark with
 */
void markStackObjects(u1 mark) {
	BEGIN;
	VALIDATE_HEAP_LISTS;
	CALL(markObjects(mark, stack, context.stackPointer));
	VALIDATE_HEAP_LISTS;
	END;
}

static heapElement* protectedHeapElement = NULL;

void heapProtect(jobject jref) {
	// This seems to be a non reentrant method... and indeed not multi threading prepared!
	if (jref != NULL) {
		protectedHeapElement = OBJECT_REF_TO_HEAP_ELEMENT(jref);
	} else {
		protectedHeapElement = NULL;
	}
}

// The mark variable - 0 shall be avoided, since this is the default value after 'new'
static u1 mark = 1;

void markAndSweep(void) {
	BEGIN;
	//	consout("Mark & Sweep BEGIN\n");
	markAndSweepCount++;

	VALIDATE_HEAP_LISTS;
	// Increment mark variable
	// When an object is new'ed, the mark is set to 0 - avoid that value as a mark value:
	mark += 2;

	// Mark objects:
	CALL(markStackObjects(mark));
	// Mark static objects
	CALL(markStaticObjects(mark));

	// Mark protected:
	if (protectedHeapElement != NULL) {
		protectedHeapElement->mark = mark;
	}
	// Sweep objects:

	//	heapElement* e = usedList;
	//	int sumsize = 0;
	//	while (e != NULL) {
	//		consout("usedList: %x size=%d mark=%d cid=%d\n", e, e->size, e->mark, e->classId);
	//		sumsize += e->size;
	//		e = e->next;
	//	}
	//	consout("total size before: %d\n", sumsize);
	//consout("Walking through heap");
	BOOL removed = TRUE;
	while (removed) {
		removed = FALSE;

		// Iterate through usedList:
		heapElement* e = usedList;
		while (e != NULL && !removed) {
			// Point to next element before it might be freed:
			// heapElement* next = e->next;
			if (e->mark != mark) {
				removed = TRUE;
				VALIDATE_HEAP_LISTS;
				CALL(heapFree(e));
				VALIDATE_HEAP_LISTS
				;
			} else {
				// Object is still in use
				e = e->next;
			}
		}
	}
	//	e = usedList;
	//	sumsize = 0;
	//	while (e != NULL) {
	//		consout("usedList: %x size=%d mark=%d cid=%d\n", e, e->size, e->mark, e->classId);
	//		sumsize += e->size;
	//		e = e->next;
	//	}
	//	consout("total size after: %d\n", sumsize);

	//	// Iterate through usedList:
	//	heapElement* e = usedList;
	//	//consout("Walking through heap");
	//	//repaint();
	//	while (e != NULL) {
	//		// Point to next element before it might be freed:
	//		heapElement* next = e->next;
	//		// Don't GC java.lang.Class - is this 'if' a hack (any object should have a reference to java.lang.Class,
	//		// but we are faking it)?
	//		// TODO Consider adding a java.lang.Class reference to Object?
	//		if (e == protectedHeapElement) {
	//			consout("Protected: %x\n", e);
	//		}
	//		if (e->classId != javaLangClassClassIndex && e->mark != mark) {
	//			VALIDATE_HEAP_LISTS;
	//			CALL(heapFree(e));
	//			VALIDATE_HEAP_LISTS
	//			;
	//		}
	//		// else: Object is still in use
	//		e = next;
	//	}


	VALIDATE_HEAP_LISTS;

	//	consout("MARK & SWEEP end\n");
	END;
}

/**
 * See .h - file
 */
void getHeapStat(heapListStat* usedStat, heapListStat* freeStat, gcStat* gc) {
	BEGIN;
	VALIDATE_HEAP_LISTS;

	freeStat->count = 0;
	freeStat->size = 0;
	heapElement* e;
	for (e = freeList; e != NULL; e = e->next) {
		freeStat->count++;
		freeStat->size += e->size;
	}

	usedStat->count = 0;
	usedStat->size = 0;
	for (e = usedList; e != NULL; e = e->next) {
		usedStat->count++;
		usedStat->size += e->size;
	}

	gc->markAndSweepCount = markAndSweepCount;
	END;
}

void dumpHeap(void) {
#ifdef TRACE
	consout("requested size = %d\n", (int) requestedSize);
	dumpUsedList();
	dumpFreeList();
#endif
}

#ifdef TRACE
static void dumpUsedList(void) {
	heapElement* hep;
	consout("Used list; mark = %d:\n", (int) mark);
	size_t total = 0;
	int count = 0;
	for (hep = usedList; hep != NULL; hep = hep->next) {
		consout("Used object: %08x %2d", hep, sizeof(heapElement));
		consout(" classId: %3d", (int) hep->classId);

		consout(" size: %3d", (int) hep->size);
		consout(" mark: %3d", (int) hep->mark);
		consout(" magic: %04x", hep->magic);

		if (hep->classId == 0) {
			consout(" (java.lang.Object)\n");
		} else if (hep->classId == javaLangStringClassIndex) {
			consout(" (java.lang.String)\n");
		} else if (hep->classId == javaLangClassClassIndex) {
			consout(" (java.lang.Class)\n");
		} else {
			consout("\n");
		}
		total += hep->size;
		count++;

		//		if (hep->classId == javaLangStringClassIndex) {
		//			jobject s = HEAP_ELEMENT_TO_OBJECT_REF(hep);
		//
		//			stackable* val = (stackable*) s;
		//			array* a = (array*) (val->operand.jref);
		//			int i;
		//			for (i = 0; i < a->length; i++) {
		//				char c = a->data[i];
		//				consout("%c", c);
		//			}
		//			consout("\n");
		//
		//		}
	}
	consout("Used *** Total size: %d; #elements = %d\n", total, count);
}

void _heapTester(void) {
	heapElement* hep;
	for (hep = usedList; hep != NULL; hep = hep->next) {
		//consout("Used object: %08x %2d", hep, sizeof(heapElement));
		if ((((int) hep) & 0xffff0000) != 0x08050000) {
			consout("Bingo! %08x\n", hep);
			consout("conext: pc=%04x sp=%04x\n", context.programCounter, context.stackPointer);
			dumpHeap();
			jvmexit(1);
		}
	}
}

static void dumpFreeList(void) {
	heapElement* hep;
	consout("Free list:\n");
	size_t total = 0;
	int count = 0;
	for (hep = freeList; hep != NULL; hep = hep->next) {
		consout("Free object: %08x", hep);

		consout(" size: %3d", (int) hep->size);

		consout("\n");
		total += hep->size;
		count++;
	}
	consout("Free *** Total size: %d; #elements = %d\n", total, count);
}
#endif
