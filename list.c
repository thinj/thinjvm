/*
 * list.c
 *
 *  Created on: Aug 17, 2012
 *      Author: hammer
 */

#include "console.h"

#include "list.h"
#include "heaplist.h"

/**
 * This method returns a header_t* that is offset from h by size * sizeof(align_t) bytes
 *
 * \param h The header to offset from
 * \param size The number of align_t to offset with
 * \return The offset header*
 */
header_t* offset_header(header_t* h, size_t size) {
	return (header_t*) (((align_t*) h) + size);
}

//void list_prepend(header_t** list, header_t* h) {
//	if (h == NULL) {
//		consout("Element is NULL\n");
//		HEAP_EXIT;
//	}
//	h->e.next = *list;
//	*list = h;
//}

void list_remove(header_t** list, header_t* prev, header_t* h) {
	header_t* l = *list;

	// Validation begin:
	// Find h in list:
	while (l != NULL && h != l) {
		l = l->e.next;
	}

	if (l == NULL) {
		consout("Element is not in list\n");
		HEAP_EXIT;
	}
	// Validation end

	// Invariant: h == l

	if (*list == h) {
		// It's the first element:
		*list = h->e.next;
	} else if (h->e.next == NULL) {
		// It's the last element, and there is an element in front of it:
		prev->e.next = NULL;
	} else {
		// It's not the first and not the last element:
		prev->e.next = h->e.next;
	}
}

/**
 * This method will try to merge the element with it's next, if these are adjacant in memory
 * \param h The element to merge
 */
static void list_merge(header_t* h) {
	if (h == NULL) {
		consout("Element is NULL\n");
		HEAP_EXIT;
	}

	header_t* next = h->e.next;
	if (next != NULL) {
		if (offset_header(h, h->e.size) == next) {
			// Merge h with next:
			h->e.next = next->e.next;
			h->e.size += next->e.size;
		}
	}
}

void list_insert_ordered(header_t** list, header_t* h) {
	if (h == NULL) {
		consout("Element is NULL\n");
		HEAP_EXIT;
	}

	h->e.next = NULL;

	// Find position in list where h shall be inserted:
	header_t* prev = NULL;
	header_t* l = *list;

	while (l != NULL && h > l) {
		prev = l;
		l = l->e.next;
	}

	if (h == l) {
		consout("Element is already in list\n");
		HEAP_EXIT;
	}

	// Invariant: l == NULL || h < l
	if (prev == NULL) {
		// h is inserted in front of list:
		h->e.next = *list;
		*list = h;
		list_merge(h);
	} else {
		prev->e.next = h;
		if (l != NULL) {
			// l != NULL && h < l && prev != NULL
			h->e.next = l;
			list_merge(h);
		}
		list_merge(prev);
	}
}

