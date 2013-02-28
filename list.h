/*
 * list.h
 *
 *  Created on: Aug 17, 2012
 *      Author: hammer
 */

#ifndef LIST_H_
#define LIST_H_

#include "types.h"

/**
 * This method returns a header_t* that is offset from h by size * sizeof(align_t) bytes
 *
 * \param h The header to offset from
 * \param size The number of align_t to offset with
 * \return The offset header*
 */
header_t* offset_header(header_t* h, size_t size);

/**
 * This method prepends an element to the list
 * \param list The list to prepend to
 * \param h The element to prepend. If NULL => system exits.
 */
void list_prepend(header_t** list, header_t* h);

/**
 * This method removes an element from the list
 * \param list The list to remove from
 * \param prev The element before the element to remove. If not found in list => system exits.
 * \param h The element to remove. If not found in list => system exits.
 */
void list_remove(header_t** list, header_t* prev, header_t* h);

/**
 * This method inserts an element into the list. The element is merged with its neighbours if
 * possible
 * \param list The list to insert in
 * \param h The element to insert. If NULL => system exits.
 */
void list_insert_ordered(header_t** list, header_t* h);

#endif /* LIST_H_ */
