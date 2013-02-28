/*
 * jni.c
 *
 *  Created on: Feb 20, 2012
 *      Author: hammer
 */

#include <string.h>
#include "jni.h"
#include "trace.h"
#include "heaplist.h"
#include "instructions.h"
#include "objectaccess.h"
#include "vmids.h"

void call_static_method(u2 referencedClassId, u2 linkId) {
	BEGIN;

	const methodInClass* mic;

	CALL(mic = getStaticMethodEntryByLinkId(referencedClassId, linkId));

	invokeCommon(mic, TRUE);

	execute();

	END;
}

void call_instance_method(jobject referencedClassObject, u2 linkId) {
	BEGIN;

	const methodInClass* mic;

	CALL(mic = getVirtualMethodEntryByLinkId(referencedClassObject, linkId));

	invokeCommon(mic, TRUE);

	execute();

	END;
}

/**
 * This function returns a pointer to the stackable within the static memory part of the
 * class identified by cls and linkId.
 * \param cls The class containing the static field
 * \param linkId The id of the member
 * \param type The expected type of the static field (for validation only)
 * \return A pointer to the stackable
 */
static stackable* GetStaticFieldStackable(jclass cls, u2 linkId, stackType type) {
	u2 classId = GetIntField(cls, LINK_ID_java_lang_Class_aClassId_I);
	const fieldInClass* fic = getFieldInClassbyLinkId(classId, linkId);

	stackable* val = GetStaticField(fic->address);

#ifdef VALIDATE_TYPE
	if (!ExceptionCheck()) {
		VALIDATE_TYPE(val->type, type);
	}
#endif

	return val;
}

void SetIntField(jobject obj, u2 linkId, jint value) {
	u2 classId = oaGetClassIdFromObject(obj);
	const fieldInClass* fic = getFieldInClassbyLinkId(classId, linkId);

	stackable val;
	val.operand.jrenameint = value;
	val.type = JAVAINT;
	PutField(obj, fic->address, &val);
}

void SetObjectField(jobject obj, u2 linkId, jobject value) {
	u2 classId = oaGetClassIdFromObject(obj);
	const fieldInClass* fic = getFieldInClassbyLinkId(classId, linkId);

	stackable val;
	val.operand.jref = value;
	val.type = OBJECTREF;
	PutField(obj, fic->address, &val);
}

void SetStaticObjectField(jclass cls, u2 linkId, jobject value) {
	u2 classId = GetIntField(cls, LINK_ID_java_lang_Class_aClassId_I);
	const fieldInClass* fic = getFieldInClassbyLinkId(classId, linkId);

	stackable val;
	val.operand.jref = value;
	val.type = OBJECTREF;
	PutStaticField(fic->address, &val);
}

static stackable* GetFieldStackable(jobject obj, u2 linkId, stackType type) {
	u2 classId = oaGetClassIdFromObject(obj);
	const fieldInClass* fic = getFieldInClassbyLinkId(classId, linkId);

	stackable* val = GetField(obj, fic->address);

#ifdef VALIDATE_TYPE
	if (!ExceptionCheck()) {
		VALIDATE_TYPE(val->type, type);
	}
#endif

	return val;
}

jobject GetStaticObjectField(jclass cls, u2 linkId) {
	stackable* val = GetStaticFieldStackable(cls, linkId, OBJECTREF);

	return val == NULL ? NULL : val->operand.jref;
}

jobject GetObjectField(jobject obj, u2 linkId) {
	stackable* val = GetFieldStackable(obj, linkId, OBJECTREF);

	return val == NULL ? NULL : val->operand.jref;
}

jint GetIntField(jobject obj, u2 linkId) {
	stackable* val = GetFieldStackable(obj, linkId, JAVAINT);

	return val == NULL ? 0 : val->operand.jrenameint;
}

BOOL ExceptionCheck(void) {
	return context.exceptionThrown;
}

jstring NewString(const jchar *value) {
	u2 size;
	getClassSize(CLASS_ID_java_lang_String, &size); // size of java.lang.String

	jobject stringObject = heapAllocObjectByStackableSize(size, CLASS_ID_java_lang_String);

	if (stringObject != NULL) {
		// Allocate char[]:
		size_t len = strlen((char*) value);

		// Avoid garbage collection of the newly allocated String object (there is no reference to it yet,
		// so a protection is necessary):
		heapProtect(stringObject, TRUE);
		jobject charArray = NewCharArray(len);
		heapProtect(stringObject, FALSE);

		if (charArray != NULL) {
			// memcpy into charArray (length and type are set):
			int i;
			for (i = 0; i < len; i++) {
				SetCharArrayElement(charArray, i, value[i]);
			}

			u2 linkId = LINK_ID_java_lang_String_value__C;
			SetObjectField(stringObject, linkId, charArray);
		}
		// else: out of mem has been thrown
	}
	return stringObject;

}

jobject AllocObject(jclass cls) {
	u2 classId = GetIntField(cls, LINK_ID_java_lang_Class_aClassId_I);
	u2 size;
	getClassSize(classId, &size);
	return heapAllocObjectByStackableSize(size, classId);
}
