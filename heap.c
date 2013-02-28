/*
 * heap.c
 *
 *  Created on: Oct 13, 2010
 *      Author: hammer
 */

#include "config.h"
#include "jni.h"
#include "jarray.h"
#include "heap.h"
#include "heaplist.h"
#include "constantpool.h"
#include "frame.h"
#include "exceptions.h"
#include "objectaccess.h"
#include "vmids.h"

// The current mark value:
static unsigned int markValue;

size_t HEAP_HEADER_SIZE;

void heapInit(align_t* heap, size_t size) {
	HEAP_HEADER_SIZE = ToAlignedSize(sizeof(header_t));

	heap_init(heap, size);
	// Never let markValue == 0:
	markValue = 1;
}

jobject heapAllocObjectByByteSize(u2 size, u2 classId) {
	header_t* h = heap_alloc(ToAlignedSize(size));
	if (h == NULL) {
		markAndSweep();
		h = heap_alloc(ToAlignedSize(size));
	}

	if (h != NULL) {
		h->e.classId = classId;
		// The mark - field is already init to 0 => it is always != markValue as long it isn't marked
		// => it can be garbage collected, and it will not by incident look like it is marked
	} else {
		//		consout("out of mem friends!\n");
		throwOutOfMemoryError();
	}

	return (jobject) h;
}

jobject heapAllocObjectByStackableSize(u2 size, u2 classId) {
	return heapAllocObjectByByteSize(size * sizeof(stackable), classId);
}

void heapProtect(jobject jref, BOOL protected) {
	header_t* h = jref;
	set_type(h, protected ? HT_PROTECTED : HT_USED);
}

/**
 * This method marks all objects referenced from the 'memory'
 * \param memory The arary of stackable to search
 * \size The number of elements in 'memory'
 */
static void mark2(stackable* memory, size_t size);

static void markObject3(jobject obj) {
	HEAP_VALIDATE;
	header_t* curList;
	header_t* nxtList;
	if (obj != NULL) {
		curList = getHeader(obj);
		curList->e.next = NULL;
	}

	nxtList = NULL;
	while (curList != NULL) {
		HEAP_VALIDATE;
		header_t* h = curList;
		curList = curList->e.next;
		h->e.next = NULL;
		obj = getObjectFromHeader(h);

		if (h->e.mark != markValue) {
			// Mark now to avoid future visits:
			h->e.mark = markValue;

			if (isObjectArray(h->e.classId)) {
				jobjectArray arr = (jobjectArray) obj;
				size_t length = GetArrayLength(arr);
				size_t i;
				for (i = 0; i < length; i++) {
					jobject element = GetObjectArrayElement(arr, i);
					if (element != NULL) {
						// Insert into next list;
						h = getHeader(element);
						// TODO hvis to elementer peger på samme object, risikerer vi så at collecte live objecsts?
						if (h->e.next == NULL) {
							if (nxtList == NULL) {
								h->e.next = h;
								nxtList = h;
							} else {
								h->e.next = nxtList->e.next;
								nxtList->e.next = h;
							}
						}
						// else: Already in list
					}
				}
			} else if (isPrimitiveValueArray(h->e.classId)) {
				// Ignore
			} else {
				// A 'normal' object with stackables:
				stackable* memory = GetObjectPayload(obj);
				u2 size;
				getClassSize(h->e.classId, &size);
				size_t i;
				for (i = 0; i < size; i++) {
					if (memory[i].type == OBJECTREF) {
						if (memory[i].operand.jref != NULL) {
							h = getHeader(memory[i].operand.jref);

							// TODO hvis to elementer peger på samme object, risikerer vi så at collecte live objecsts?
							if (h->e.next == NULL) {
								if (nxtList == NULL) {
									h->e.next = h;
									nxtList = h;
								} else {
									h->e.next = nxtList->e.next;
									nxtList->e.next = h;
								}
							}
							// else: Already in list

						}
					}
				}
			}
		}
		// else: Already marked

		if (curList == NULL) {
			curList = nxtList;
			nxtList = NULL;
		}

		HEAP_VALIDATE;
	}
}

static void mark2(stackable* memory, size_t size) {
	HEAP_VALIDATE;
	size_t i;
	for (i = 0; i < size; i++) {
		if (memory[i].type == OBJECTREF) {
			if (memory[i].operand.jref != NULL) {
				markObject3(memory[i].operand.jref);
			}
		}
	}HEAP_VALIDATE;
}

static stackable* sGetThreadStack(jobject stackThread) {
	// Get the aStack attribute the thread identified by 'stackThread':
	jbyteArray ba = GetObjectField(stackThread, LINK_ID_java_lang_Thread_aStack__B);

	// Set the stack pointer to the byte[]:
	return (stackable*) jaGetArrayPayLoad(ba);
}

static void sGetThreadContext(jobject stackThread, contextDef* contp) {
	// Get the aContext attribute the thread identified by 'stackThread':
	jbyteArray ba = GetObjectField(stackThread, LINK_ID_java_lang_Thread_aContext__B);

	GetByteArrayRegion(ba, 0, sizeof(contextDef), (jbyte*) contp);
}

void markAndSweep(void) {
	//	consoutli("Mark & Sweep\n");
	//	heap_dump();
	HEAP_VALIDATE;
	// The mark field in header_t is MARK_BIT_SIZE bits only. Avoiding the value '0' will
	// ensure that a newly created, not protected object will always != markValue
	// => it can be collected:
	markValue = (markValue + 2) & ((1 << MARK_BIT_SIZE) - 1);

	// Mark:
	mark2(staticMemory, staticMemorySize);

	if (frIsSchedulingEnabled()) {
		// Iterate through all threads:
		jclass threadClass = getJavaLangClass(CLASS_ID_java_lang_Thread);
		jobject stackThread = GetStaticObjectField(threadClass,
				LINK_ID_java_lang_Thread_aAllThreads_Ljava_lang_Thread_);

		jobject currentThread = GetStaticObjectField(threadClass,
				LINK_ID_java_lang_Thread_aCurrentThread_Ljava_lang_Thread_);

		while (stackThread != NULL) {
			stackable* stack = sGetThreadStack(stackThread);
			int stackPointer;
			if (stackThread != currentThread) {
				contextDef cp;
				sGetThreadContext(stackThread, &cp);
				stackPointer = cp.stackPointer;
			} else {
				// The context contained in the thread is only updated during thread switching:
				stackPointer = context.stackPointer;
			}
			mark2(stack, stackPointer);

			// Next thread:
			stackThread = GetObjectField(stackThread,
					LINK_ID_java_lang_Thread_aNextThread_Ljava_lang_Thread_);
		}
	} else {
		// The java.lang.Thread.<clinit> hasn't finished yet, so only a single thread is running:
		mark2(getStack(), context.stackPointer);
	}
	// Sweep heap:
	heap_sweep(markValue);

	//	heap_dump();
	//	consoutli("End Of Mark & Sweep\n");

	HEAP_VALIDATE;
}

void getHeapStat(heapListStat* usedStat, heapListStat* freeStat, gcStat* gc) {
	consout("not impl");
	jvmexit(1);
}

