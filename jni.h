/*
 * jni.h
 *
 *  Created on: Mar 18, 2011
 *      Author: hammer
 */

#ifndef JNI_H_
#define JNI_H_

#include "types.h"
#include "frame.h"
#include "heap.h"

#define JNIEXPORT extern
#define JNICALL

#define JNIEnv contextDef

/**
 * This method calls a static method identified by 'referencedClassId' and 'linkId'. These two parameters
 * are used as lookup keys in the table 'allMethodsInAllClasses' where they shall match 'classId' and
 * 'linkId', respectively.
 *
 * \param referencedClassId The referenced class
 * \param linkId Identifies the method within the referenced class
 */
void call_static_method(u2 referencedClassId, u2 linkId);

/**
 * This method calls an instance method.
 *
 * \param referencedClassObject The referenced class. The classId of this instance is used when looking up
 * the target method in the method table 'allMethodsInAllClasses'.
 * \param linkId Identifies the method within the referenced class
 */
void call_instance_method(jobject referencedClassObject, u2 linkId);

/**
 * This function returns the Object element at position index from the array
 * \param array The array from where the Object is looked up
 * \param index The position in the array
 * \return The Object
 */
jobject GetObjectArrayElement(jarray array, size_t index);

/**
 * This function sets the Object Array element
 * \param array The array to modify
 * \param index The array index
 * \param value The value to set at position 'index'
 */
void SetObjectArrayElement(jarray array, size_t index, jobject value);

/**
 * \return The length of the array
 */
size_t GetArrayLength(jarray array);

/**
 * This method allocates an array on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \param init All elements are initialized with this value
 * \return The allocated object or NULL, if out of mem (OutOfMemException has been thrown, in this case)
 */
jobjectArray NewObjectArray(jint count, u2 elementClassId, jobject init);

/**
 * This method allocates an array of chars on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \return The allocated object or NULL, if out of mem (OutOfMemException has been thrown, in this case)
 */
jcharArray NewCharArray(size_t len);

/**
 * This method allocates an array of ints on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \return The allocated object or NULL, if out of mem (OutOfMemException has been thrown, in this case)
 */
jintArray NewIntArray(size_t len);

/**
 * This method allocates an array of longs on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \return The allocated object or NULL, if out of mem (OutOfMemException has been thrown, in this case)
 */
jlongArray NewLongArray(size_t len);

/**
 * This method allocates an array of booleans on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \return The allocated object or NULL, if out of mem (OutOfMemException has been thrown, in this case)
 */
jbooleanArray NewBooleanArray(size_t len);

/**
 * This method allocates an array of bytes on the heap
 * \param count The number of elements
 * \param elementClassId The id of array elements
 * \return The allocated object or NULL, if out of mem (OutOfMemException has been thrown, in this case)
 */
jbyteArray NewByteArray(size_t len);

/**
 * This function returns the boolean element at position index from the array
 * \param array The array from where the boolean is looked up
 * \param index The position in the array
 * \return The boolean
 */
jboolean GetBooleanArrayElement(jarray array, size_t index);
/**
 * This function returns the byte element at position index from the array
 * \param array The array from where the byte is looked up
 * \param index The position in the array
 * \return The byte
 */
jbyte GetByteArrayElement(jarray array, size_t index);
/**
 * This function returns the char element at position index from the array
 * \param array The array from where the byte is looked up
 * \param index The position in the array
 * \return The value
 */
jchar GetCharArrayElement(jarray array, size_t index);

/**
 * This function returns the int element at position index from the array
 * \param array The array from where the value is looked up
 * \param index The position in the array
 * \return The value
 */
jint GetIntArrayElement(jarray array, size_t index);

/**
 * This function returns the long element at position index from the array
 * \param array The array from where the value is looked up
 * \param index The position in the array
 * \return The value
 */
jint GetLongArrayElement(jarray array, size_t index);

/**
 * This function sets the Boolean Array element
 * \param array The array to modify
 * \param index The array index
 * \param value The value to set at position 'index'
 */
void SetBooleanArrayElement(jarray array, size_t index, BOOL value);

/**
 * This function sets the Byte Array element
 * \param array The array to modify
 * \param index The array index
 * \param value The value to set at position 'index'
 */
void SetByteArrayElement(jarray array, size_t index, jbyte value);

/**
 * This function sets the int Array element
 * \param array The array to modify
 * \param index The array index
 * \param value The value to set at position 'index'
 */
void SetIntArrayElement(jarray array, size_t index, jint value);

/**
 * This function sets the long Array element
 * \param array The array to modify
 * \param index The array index
 * \param value The value to set at position 'index'
 */
void SetLongArrayElement(jarray array, size_t index, jlong value);

/**
 * This function sets the char Array element
 * \param array The array to modify
 * \param index The array index
 * \param value The value to set at position 'index'
 */
void SetCharArrayElement(jarray array, size_t index, jchar value);

/**
 * This method sets the instance attribute identified by linkId in
 * the object obj to the value val
 * \param obj The object to modify
 * \param linkId The link id of the field
 * \param value The value to set
 */
void SetIntField(jobject obj, u2 linkId, jint value);

/**
 * This method sets the instance attribute identified by linkId in
 * the object obj to the value val
 * \param obj The object to modify
 * \param linkId The link id of the field
 * \param value The value to set
 * \throws NullPointerException if obj == NULL
 */
void SetObjectField(jobject obj, u2 linkId, jobject value);

/**
 * This method sets the instance attribute identified by linkId in
 * the class cls to the value val
 * \param cls The class to modify
 * \param linkId The link id of the field
 * \param value The value to set
 */
void SetStaticObjectField(jclass cls, u2 linkId, jobject value);

/**
 * This method gets the instance attribute identified by linkId in
 * the object obj
 * \param obj The object to get from
 * \param linkId The link id of the field
 * \return value The read value
 * \throws NullPointerException if obj == NULL
 */
jobject GetObjectField(jobject obj, u2 linkId);

/**
 * This method gets the class attribute identified by linkId in
 * the class cls
 * \param cls The cls to get from
 * \param linkId The link id of the field
 * \return value The read value
 */
jobject GetStaticObjectField(jclass cls, u2 linkId);

/**
 * This method gets the instance attribute identified by linkId in
 * the object obj
 * \param obj The object to get from
 * \param linkId The link id of the field
 * \return value The read value
 * \throws NullPointerException if obj == NULL
 */
jint GetIntField(jobject obj, u2 linkId);

/**
 * This method returns true if an exception has been thrown
 * \return true if an exception has been thrown; false otherwise
 */
BOOL ExceptionCheck(void);

/**
 * This function allocates a String with the contents specified by the '\0' - terminated
 * argument 'chars'
 * \param chars The text string
 * \return The allocated java.lang.String or null, if out of mem
 * \throws OutOfMemException
 */
jstring NewString(const jchar *chars);

/**
 * This method copies the contents of buf into array starting at position 'start' in 'array', reading
 * 'len' jbytes from 'buf'
 * \param array The array to copy into
 * \param start The offset in array
 * \param len The number of bytes to copy
 * \buf The pointer to the buffer to copy from
 * \throws NullPointerException if array == NULL
 * \throws ArrayIndexOutOfBoundsException if start < 0 or start+len > length of array
 */
void SetByteArrayRegion(jbyteArray array, jsize start, jsize len, jbyte *buf);

/**
 * This method copies the contents of array starting at position 'start' in 'array' into 'buf', reading
 * 'len' jbytes from 'array'
 * \param array The array to copy from
 * \param start The offset in array
 * \param len The number of bytes to copy
 * \buf The pointer to the buffer to copy to
 * \throws NullPointerException if array == NULL
 * \throws ArrayIndexOutOfBoundsException if start < 0 or start+len > length of array
 */
void GetByteArrayRegion(jbyteArray array, jsize start, jsize len, jbyte *buf);

/**
 * This function allocates an instance of the class identified by cls
 * \param cls Identifies the class of which an instance is requested
 * \return The allocated instance or NULL, if out of mem
 * \throws OutOfMemException
 */
jobject AllocObject(jclass cls);

#endif /* JNI_H_ */
