/*
 * objectaccess.c
 *
 *  Created on: Sep 20, 2012
 *      Author: hammer
 */

#include "objectaccess.h"
#include "heaplist.h"
#include "heap.h"
#include "constantpool.h"
#include "exceptions.h"

void PutField(jobject obj, u2 address, stackable* value) {
	HEAP_VALIDATE;

	if (obj != NULL) {
		stackable* objectPayload = (stackable*) GetObjectPayload(obj);
		objectPayload += address;
		*objectPayload = *value;
	} else {
		throwNullPointerException();
	}

	HEAP_VALIDATE;
}

stackable* GetField(jobject obj, u2 address) {
	HEAP_VALIDATE;

	stackable* objectPayload;
	if (obj != NULL) {
		objectPayload = (stackable*) GetObjectPayload(obj);
		objectPayload += address;
	} else {
		objectPayload = NULL;
		throwNullPointerException();
	}
	return objectPayload;
}

void PutStaticField(u2 address, stackable* value) {
	HEAP_VALIDATE;

	stackable* objectPayload = staticMemory;
	objectPayload += address;
	*objectPayload = *value;

	HEAP_VALIDATE;
}

stackable* GetStaticField(u2 address) {
	HEAP_VALIDATE;

	stackable* objectPayload;
	objectPayload = staticMemory;
	objectPayload += address;

	return objectPayload;
}

/**
 * This method allocates an instance of the specified object. No constructors are invoked.
 *
 * \param classId Identifies the class to instantiate
 * \return The allocated object or null, if out of mem has been thrown
 */
jobject newObject(u2 classId) {
	// TODO Move to jni.c and add correct semantics (by calling the constructor method as well)
	u2 size;
	getClassSize(classId, &size);

	jobject jref = heapAllocObjectByStackableSize(size, classId);

	return jref;
}

/**
 * This function returns the control header of the object. It's the callers responsibility to ensure
 * that obj != NULL
 * \param obj The jobject for which the header_t shall be returned
 * \return The header_t* for the jobject
 */
header_t* getHeader(jobject obj) {
	return (header_t*) obj;
}

/**
 * This function returns the object refered by the header_t* h. It's the callers responsibility to ensure
 * that h really refers to a jobject
 * \param h The header_t* for which the refered jobject is requested
 * \return The refered jobject
 */
jobject getObjectFromHeader(header_t* h) {
	return (jobject) h;
}

/**
 * This function returns the class id of the object. It's the callers responsibility to ensure
 * that obj != NULL
 * \param obj The jobject for which the class id shall be returned
 * \return The class id for the jobject
 */
u2 oaGetClassIdFromObject(jobject obj) {
	return getHeader(obj)->e.classId;
}
