/*
 * heaplist.h
 *
 *  Created on: Aug 17, 2012
 *      Author: hammer
 */

#ifndef HEAPLIST_H_
#define HEAPLIST_H_

#include <stdlib.h>
#include "types.h"
#include "list.h"

/**
 * This struct contains stat about a heap list
 */
typedef struct __heapstat_t {
	// The number of elements in list:
	int count;

	// The number of bytes in the list:
	size_t size;
} heapstat_t;

/**
 * This function initialises the heap
 * \param memory The pointer to the location in RAM where the heap resides
 * \length The size of the memory - in counts of align_t, not bytes
 */
void heap_init(align_t* memory, size_t length);

/**
 * This function allocates a memory area
 * \size The size of the memory chunk in number of align_t, no bytes
 * \return The allocated memory chunk or NULL, if out of memory
 */
header_t* heap_alloc(size_t size);

/**
 * This method frees the element h
 * \param h The element to free. If NULL => exit is called.
 */
void heap_free(header_t* h);

/**
 * This function performs sanity check of the heap
 * \param file The name of the calling file
 * \param lineno The line number in the calling file
 */
void heap_validate(const char* file, const int lineno);
//#define HEAP_VALIDATE heap_validate(__FILE__, __LINE__)
#define HEAP_VALIDATE

/**
 * This function collects info about heap usage
 * \param used The used heap memory info
 * \param used The free heap memory info
 */
void heap_stat(heapstat_t* used, heapstat_t* free);

/**
 * This function dumps the heap on stdout
 */
void heap_dump(void);

/**
 * This function removes all used elements at the heap having a mark != markValue
 * \param markValue The current mark value to test against.
 */
void heap_sweep(unsigned int markValue);

/**
 * This function prints src file and line number and exits.
 * \param file The name of the calling file
 * \param lineno The line number in the calling file
 */
void heap_exit(const char* file, const int lineno);
#define HEAP_EXIT heap_exit(__FILE__, __LINE__)

/**
 * This function returns the number bytes used on heap
 * \return The number bytes used on heap
 */
int heap_used();

/**
 * This function returns the size of the heap in byte
 * \return The size of the heap in byte
 */
int heap_max();


//------------------------------------------------------------------
// type field access
//------------------------------------------------------------------
/**
 * This function returns != 0, if the header 'h' is of type 'type'
 * \param h The header to test
 * \param type The type to test against
 * \return returns != 0, if the header 'h' is of type 'type'; 0 otherwise
 */
int is_type(header_t* h, hdrtype_t type);

/**
 * This function sets the type of the header
 * \param h The header to alter
 * \param type The type to set
 */
void set_type(header_t* h, hdrtype_t type);

/**
 * This function returns the type of the header
 * \param h The header
 * \return The type of the header
 */
hdrtype_t get_type(header_t* h);

/**
 * This function returns a textual representation of a header type
 * \param type The type to convert to text
 * \return The textual representation
 */
const char * type_to_str(hdrtype_t type);

/**
 * \return The size of the heap in bytes
 */
size_t hlGetHeapSizeInBytes();

#endif /* HEAPLIST_H_ */
