/*
 * thinjvm.h
 *
 *  Created on: Jun 22, 2012
 *      Author: hammer
 */

#ifndef THINJVM_H_
#define THINJVM_H_

#include <stdlib.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * This function starts the VM and will never return.
 * \param heap A pointer to the memory area where the heap will be placed
 * \param heapSize The size of the heap area (in chunks of align_t)
 * \param stackSize The size of the stack (in bytes)
 */
void thinjvm_run(align_t* heap, size_t heapSize, size_t stackSize);

/**
 * This function shall exit the vm
 */
void thinjvm_exit(int exit_code);

#ifdef  __cplusplus
}
#endif


#endif /* THINJVM_H_ */
