/*
 * exceptions.h
 *
 *  Created on: Sep 23, 2012
 *      Author: hammer
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

/**
 * This method throws an OutOfMemoryError
 * \throws Unconditionally an OutOfMemoryError
 */
void throwOutOfMemoryError(void);

/**
 * \throws Unconditionally a NullPointerException
 */
void throwNullPointerException(void);

/**
 * \param exception The exception to throw
 * \throws Unconditionally the exception
 */
void throwException(jobject exception);

/**
 * \param index The offending index
 * \param arrayLength The length of the arary
 * \throws ArrayIndexOutOfBoundsException unconditionally
 */
void throwArrayIndexOutOfBoundsException(jint index, jint arrayLength);

/**
 * \param classId_S
 * \param classId_T
 * \throws ClassCastException unconditionally
 */
void throwClassCastException(u2 classId_S, u2 classId_T);

/**
 * \throws NegativeArraySizeException unconditionally
 */
void throwNegativeArraySizeException();

/**
 * \param cause The cause of the exception
 * \throws NegativeArraySizeException unconditionally
 */
void throwArithmeticException(const char* cause);


#endif /* EXCEPTIONS_H_ */
