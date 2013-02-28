/*
 * objectaccess.h
 *
 *  Created on: Sep 20, 2012
 *      Author: hammer
 */

#ifndef OBJECTACCESS_H_
#define OBJECTACCESS_H_

#include "types.h"
#include "operandstack.h"

/**
 * This method updates a field in an object
 * \param obj The object to update
 * \param address The address of the field (field is a stackable)
 * \param value The new value
 * \throws NullPointerException if obj == null
 */
void PutField(jobject obj, u2 address, stackable* value);

/**
 * This method returns a pointer to a value in an object
 * \param obj The object to read from
 * \param address The address of the field (field is a stackable)
 * \return The pointer to the value
 * \throws NullPointerException if obj == null
 */
stackable* GetField(jobject obj, u2 address);

/**
 * This method returns a pointer to a value in a class
 * \param address The address of the field (field is a stackable)
 * \return The pointer to the value
 */
stackable* GetStaticField(u2 address);

/**
 * This method updates a field in a class
 * \param address The address of the field (field is a stackable)
 * \param value The new value
 */
void PutStaticField(u2 address, stackable* value);

/**
 * This method allocates an instance of the specified object. No constructors are invoked.
 *
 * \param classId Identifies the class to instantiate
 * \return The allocated object or null, if out of mem has been thrown
 */
jobject newObject(u2 classId);

/**
 * This method creates and allocates a java.lang.String object and returns the created Object
 * \param value The constant string to initiate with
 * \return The allocated object
 */
jobject allocJavaLangString(const char* value);

/**
 * This function returns the control header of the object. It's the callers responsibility to ensure
 * that obj != NULL
 * \param obj The jobject for which the header_t shall be returned
 * \return The header_t* for the jobject
 */
header_t* getHeader(jobject obj);

/**
 * This function returns the object refered by the header_t* h. It's the callers responsibility to ensure
 * that h really refers to a jobject
 * \param h The header_t* for which the refered jobject is requested
 * \return The refered jobject
 */
jobject getObjectFromHeader(header_t* h);

/**
 * This function returns the class id of the object. It's the callers responsibility to ensure
 * that obj != NULL
 * \param obj The jobject for which the class id shall be returned
 * \return The class id for the jobject
 */
u2 oaGetClassIdFromObject(jobject obj);

#endif /* OBJECTACCESS_H_ */
