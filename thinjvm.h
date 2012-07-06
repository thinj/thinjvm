/*
 * thinjvm.h
 *
 *  Created on: Jun 22, 2012
 *      Author: hammer
 */

#ifndef THINJVM_H_
#define THINJVM_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function starts the VM and will never return.
 * \param heap A pointer to the memory area where the heap will be placed
 * \param heapSize The size of the heap area
 */
void thinjvm_run(void* heap, size_t heapSize);

/**
 * This function shall exit the vm
 */
void thinjvm_exit(int exit_code);

#ifdef  __cplusplus
}
#endif


#endif /* THINJVM_H_ */
