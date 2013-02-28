/*
 * heaplist.c
 *
 *  Created on: Aug 17, 2012
 *      Author: hammer
 */

#include  <stdio.h>
#include <string.h>

#include "console.h"
#include "heaplist.h"
#include "config.h"

// The buffer with the heap:
static header_t* heap;

// The size of the heap in count of sizeof(align_t):
static size_t heap_size;

static header_t* free_list;

static header_t* to_header(align_t* a) {
	return (header_t*) a;
}

size_t hlGetHeapSizeInBytes() {
	return heap_size * sizeof(align_t);
}

//------------------------------------------------------------------
// type field access
//------------------------------------------------------------------
int is_type(header_t* h, hdrtype_t type) {
	return h->e.type == type;
}

void set_type(header_t* h, hdrtype_t type) {
	h->e.type = type;
}

hdrtype_t get_type(header_t* h) {
	return h->e.type;
}

const char * type_to_str(hdrtype_t type) {
	switch (type) {
	case HT_USED:
		return "USED";
	case HT_FREE:
		return "FREE";
	case HT_PROTECTED:
		return "RESV";
	default:
		return NULL;
	}
}

//------------------------------------------------------------------
// mark field access
//------------------------------------------------------------------

/**
 * This method initialises an element
 * \param h The element to initialise
 * \param length The size of the element (in number of header_t, not bytes)
 * \param hdrtyp The type of the element
 */
static void init_element(header_t *h, size_t length, hdrtype_t hdrtyp) {
	memset(h, 0, sizeof(align_t) * length);
	set_type(h, hdrtyp);
	h->e.next = NULL;
	h->e.size = length;
}

void heap_init(align_t* memory, size_t length) {
	heap = to_header(memory);
	heap_size = length;
	init_element(heap, length, HT_FREE);
	free_list = heap;

	HEAP_VALIDATE;
}

void heap_validate(const char* file, const int lineno) {
	//	consout("%04x !\n", context.programCounter);
	size_t used_m = 0;
	size_t free_m = 0;

	size_t free_l = 0;

	// Traverse through heap memory:
	size_t offset = 0;
	while (offset < heap_size) {
		header_t* h = offset_header(heap, offset);

		if (is_type(h, HT_FREE)) {
			free_m += h->e.size;
		} else if (is_type(h, HT_USED) || is_type(h, HT_PROTECTED)) {
			used_m += h->e.size;
		} else {
			consout("Heap corrupted (magic)\n");
			heap_dump();
			heap_exit(file, lineno);
		}

		// Advance pointer using size of element:
		offset += h->e.size;
	}

	if (used_m + free_m != heap_size) {
		consout("Heap corrupted (size)\n");
		heap_exit(file, lineno);
	}

	// Validate free list:
	if (free_list != NULL) {
		header_t* h = free_list;
		header_t* prev = NULL;

		free_l = 0;
		while (h != NULL) {
			if (is_type(h, HT_FREE)) {
				free_l += h->e.size;
			} else {
				consout("Heap corrupted (free magic)\n");
				heap_exit(file, lineno);
			}
			if (prev != NULL && h < prev) {
				consout("Heap corrupted (free list order)\n");
				heap_exit(file, lineno);
			}
			prev = h;
			h = h->e.next;
		}
	}

	if (free_l != free_m) {
		consout("Heap corrupted (free size)\n");
		heap_exit(file, lineno);
	}
}

void heap_sweep(unsigned int markValue) {
	HEAP_VALIDATE;

	// TODO Clear free list and rebuild during this sweep ?

	header_t* h = heap;
	size_t offset = 0;
	// previous is != NULL, if the previous was free entry:
	header_t* previous = NULL;

	while (offset < heap_size) {
		h = offset_header(heap, offset);
		if (is_type(h, HT_FREE)) {
			previous = h;
			offset += h->e.size;
		} else if (is_type(h, HT_USED)) {
			if (h->e.mark != markValue) {
				//--------------------------
				// Garbage collect this one:
				//--------------------------
				size_t previousSize = 0;
				if (previous != NULL) {
					// h shall be merged with previous, remember the size of previous:
					previousSize = previous->e.size;
				}

				//				consout("Collecting %p (%d)\n", h, (int)h->e.classId);
				// This call might merge with previous and next element on heap:
				heap_free(h);

				if (previous != NULL) {
					// h might have been merged with previous; don't count previous size twice
					// (previous->e.size might have been increased with the size of h during the merge):
					offset += previous->e.size - previousSize;
					// don't change previous
				} else {
					offset += h->e.size;
					previous = h;
				}
			} else {
				offset += h->e.size;
				previous = NULL;
			}
		} else if (is_type(h, HT_PROTECTED)) {
			previous = NULL;
			offset += h->e.size;
		} else {
			consout("Heap corrupted (magic)\n");
			HEAP_EXIT;
		}
	}

	HEAP_VALIDATE;
}

void heap_free(header_t* h) {
	HEAP_VALIDATE;

	if (h == NULL) {
		consout("Element is NULL\n");
		HEAP_EXIT;
	}

	if (!is_type(h, HT_USED) && !is_type(h, HT_PROTECTED)) {
		consout("Magic\n");
		HEAP_EXIT;
	}

	set_type(h, HT_FREE);
	list_insert_ordered(&free_list, h);

	HEAP_VALIDATE;
}

header_t* heap_alloc(size_t size) {
	header_t* h = free_list;
	header_t* best_fit = NULL;
	header_t* prev_best_fit = NULL;
	header_t* prev = NULL;

	HEAP_VALIDATE;

	// Also allocate mem for header:
	size += HEAP_HEADER_SIZE;

	// Find best candidate:
	while (h != NULL) {
		if (h->e.size >= size) {
			// h fulfils the request, but is it the best:
			if (best_fit != NULL) {
				if (h->e.size < best_fit->e.size) {
					prev_best_fit = prev;
					best_fit = h;
				}
			} else {
				prev_best_fit = prev;
				best_fit = h;
			}
		}
		prev = h;
		h = h->e.next;
	}

	if (best_fit != NULL) {
		// remove from free list:
		list_remove(&free_list, prev_best_fit, best_fit);

		size_t remaining_size = best_fit->e.size - size;
		size_t best_fit_size;

		if (remaining_size > HEAP_HEADER_SIZE) {
			// The best_fit is too large; split into two parts:
			header_t* remaining = offset_header(best_fit, size);
			init_element(remaining, remaining_size, HT_FREE);

			// Return to free list:
			list_insert_ordered(&free_list, remaining);

			// Adjust the size of best fit:
			best_fit_size = size;
		} else {
			// No split:
			best_fit_size = best_fit->e.size;
		}
		// else: Perfect fit
		init_element(best_fit, best_fit_size, HT_USED);
	}
	// else: Out of mem
	HEAP_VALIDATE;

	return best_fit;
}

void heap_exit(const char* file, const int lineno) {
	consout("exit at: %s:%d\n", file, lineno);
	jvmexit(1);
}

void heap_stat(heapstat_t* used, heapstat_t* free) {
	HEAP_VALIDATE;

	used->size = 0;
	used->count = 0;
	free->size = 0;
	free->count = 0;

	size_t offset = 0;
	while (offset < heap_size) {
		header_t* h = offset_header(heap, offset);

		if (is_type(h, HT_FREE)) {
			free->size += h->e.size;
			free->count++;
		} else if (is_type(h, HT_USED) || is_type(h, HT_PROTECTED)) {
			used->size += h->e.size;
			used->count++;
		} else {
			consout("Heap corrupted (magic)\n");
			HEAP_EXIT;
		}
		offset += h->e.size;
	}
}

int heap_used() {
	HEAP_VALIDATE;

	int size = 0;

	size_t offset = 0;
	while (offset < heap_size) {
		header_t* h = offset_header(heap, offset);

		if (is_type(h, HT_USED) || is_type(h, HT_PROTECTED)) {
			size += h->e.size;
		}
		offset += h->e.size;
	}

	return size * sizeof(align_t);
}

int heap_max() {
	return heap_size * sizeof(align_t);
}

static void sDumpObject(header_t* h) {
	if (0 <= h->e.classId && h->e.classId < numberOfAllClassInstanceInfo) {
		if (isObjectArray(h->e.classId)) {
			jarray oa = (jarray) getObjectFromHeader(h);
			size_t length = GetArrayLength(oa);
			size_t i;
			for (i = 0; i < length; i++) {
				jobject obj = GetObjectArrayElement(oa, i);
				header_t* oh = getHeader(obj);
				consout("    [%d] %p\n", i, oh);
			}
		}
	}
	// else: Class Id out of range; can't dump
}

void heap_dump() {
	//HEAP_VALIDATE;

	size_t offset = 0;
	consout("Address   Type  Size Mark  CID           Next\n");
	consout("=============================================\n");
	int usedElements = 0;
	int freeElements = 0;
	int protElement = 0;
	size_t usedSize = 0;
	size_t freeSize = 0;
	size_t protSize = 0;
	while (offset < heap_size) {
		header_t* h = offset_header(heap, offset);

		if (get_type(h) == HT_USED) {
			usedElements++;
			usedSize += h->e.size;
		}
		if (get_type(h) == HT_FREE) {
			freeElements++;
			freeSize += h->e.size;
		}
		if (get_type(h) == HT_PROTECTED) {
			protElement++;
			protSize += h->e.size;
		}
		consout("%p %s %4d   %2d %4d  ->%10p\n", h, type_to_str(get_type(h)), h->e.size, h->e.mark,
				h->e.classId, h->e.next);
		if (get_type(h) == HT_PROTECTED || get_type(h) == HT_USED) {
			sDumpObject(h);
		}

		offset += h->e.size;
	}
	consout("\n");
	consout("Total %s: %d/%d\n", type_to_str(HT_USED), usedSize, usedElements);
	consout("Total %s: %d/%d\n", type_to_str(HT_FREE), freeSize, freeElements);
	consout("Total %s: %d/%d\n", type_to_str(HT_PROTECTED), protSize, protElement);
}
