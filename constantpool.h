/*
 * constantpool.h
 *
 *  Created on: Aug 23, 2010
 *      Author: hammer
 */

#ifndef CONSTANTPOOL_H_
#define CONSTANTPOOL_H_
#include "types.h"
#include "operandstack.h"

#define JAVA_LANG_OBJECT_CLASS_ID 0

typedef struct __memberReference {
	// The class id of the referencing class:
	u2 referencingClassId;
	// The constant pool index in the referencing class:
	u2 index;
	// The referenced class:
	u2 referencedClassId;
	// The reference to the member
	u2 linkId;
	// The number of arguments (including 'this) (only applicable for methods):
	u1 numberOfArguments;
} memberReference;

typedef struct __methodInClass {
	// The containing class
	u2 classId;
	// The id of the member
	u2 linkId;
	// Address of the method (native methods have the address 0):
	u2 codeOffset;
	// The length of the method code (native method has code length 0):
	u2 codeLength;
	// The number of local variables including the number of arguments:
	u2 numberOfLocalVariables;
	// The number of arguments
	u2 numberOfArguments;
	// if > 0: This is a native method; (nativeIndex - 1) is the index into the call-table:
	u2 nativeIndex;
} methodInClass;

typedef struct __exceptionHandler {
	// The containing class:
	u2 classId;

	// The class Id of the exception type:
	u2 exceptionClassId;

	// Scope start address:
	u2 startPC;

	// Scope end (exclusive) address:
	u2 endPC;

	// Handler address:
	u2 handlerPC;
} exceptionHandler;

/**
 * A function pointer to an (encapsulated) native method
 */
typedef void (*nativeJumpTableEntry)(void);

typedef struct __fieldInClass {
	// The containing class
	u2 classId;
	// The id of the member
	u2 linkId;
	u2 address;
	u1 size;
} fieldInClass;

typedef struct __integerConstantReference {
	u2 classId;
	// Index into constant pool of the referencing class:
	u2 constantPoolIndex;
	jint value;
} integerConstantReference;

typedef struct __floatConstantReference {
	u2 classId;
	// Index into constant pool of the referencing class:
	u2 constantPoolIndex;
	jfloat value;
} floatConstantReference;

typedef struct __doubleConstantReference {
	u2 classId;
	// Index into constant pool of the referencing class:
	u2 constantPoolIndex;
	jdouble value;
} doubleConstantReference;

typedef struct __stringConstantReference {
	u2 classId;
	// Index into constant pool of the referencing class:
	u2 constantPoolIndex;
	char* value;
} stringConstantReference;

typedef struct __classReference {
	// The class id of the referencing class:
	u2 classId;

	// Index into constant pool of the referencing class:
	u2 constantPoolIndex;

	// The class id of the referenced class:
	u2 targetClassId;
} classReference;

typedef struct __lineNumberInfoDef {
	// The class id of the referencing class:
	u2 classId;
	// The first code address
	codeIndex startPC;
	// The line number in source file:
	u2 lineNumber;
	// The method name;
	char* methodName;
} lineNumberInfoDef;

typedef struct __classInstanceInfoDef {
	// The class id of the class:
	u2 classId; // classId == 0 <=> it's java.lang.Object
	// The class id of the super class:
	u2 superClassId; // if superClassId == 0 => it's java.lang.Object
	// The size of an instance
	u2 instanceSize;
	// The index of the list of implemented interfaces:
	size_t interface_start;
	// The number of implemented interfaces:
	size_t interface_count;
	// The type of the class:
	CLASS_TYPE type;
	// The class id of an array element - is only defined when type == CT_OBJECT_ARRAY:
	u2 elementClassId;
	// The size of a single element in the array. Makes sense for arrays only:
	size_t elementSize;
} classInstanceInfoDef;

typedef struct __arrayInfoDef {
	// The class id of the array class:
	u2 classId;
	// The size of a single element in bytes:
	u1 sizeInBytes;
	// The 'atype' as defined in the JVM spec:
	ARRAY_TYPE arrayType;
} arrayInfoDef;

typedef enum __BuildinDependency_e {
	NULL_POINTER_EXCEPTION_CLASS, NULL_POINTER_EXCEPTION_CONSTRUCTOR,

	ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS, ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CONSTRUCTOR,

	ARITHMETIC_EXCEPTION_CLASS, ARITHMETIC_EXCEPTION_CONSTRUCTOR,

	OUT_OF_MEMORY_ERROR_CLASS, OUT_OF_MEMORY_ERROR_CONSTRUCTOR, OUT_OF_MEMORY_ERROR_GET_INSTANCE,

	NEGATIVE_ARRAY_SIZE_EXCEPTION_CLASS, NEGATIVE_ARRAY_SIZE_EXCEPTION_CONSTRUCTOR,

	CLASS_CAST_EXCEPTION_CLASS, CLASS_CAST_EXCEPTION_CONSTRUCTOR
} BuildinDependency_e;

typedef struct __BuildinClassDependency_s {
	// The reference from lib - file:
	BuildinDependency_e libRef;

	// The Class Id:
	u2 classId;
} BuildinClassDependency_s;

typedef struct __BuildinMemberDependency_s {
	// The reference from lib - file:
	BuildinDependency_e libRef;

	// The Link Id:
	u2 linkId;
} BuildinMemberDependency_s;

typedef struct __constantPool {
	// All method references in this constant pool:
	const memberReference* const methodReferences;

	// The number of method references:
	const size_t numberOfMethodReferences;

	// All method in class:
	const methodInClass* const methods;

	// The number of methods:
	const size_t numberOfMethods;
} constantPool;

/**
 * The size of all JVM code:
 */
extern const u2 codeSize;
extern const u1 code[];

/**
 * Array classes info:
 */
extern const u2 numberofAllArrayInfo;
extern const arrayInfoDef arrayInfo[];

extern const int staticMemorySize;
extern stackable staticMemory[];

extern const classInstanceInfoDef const allClassInstanceInfo[];
extern const u2 numberOfAllClassInstanceInfo;

extern const u2 numberOfAllClassReferences;
extern const classReference const allClassReferences[];

// The classId of java.lang.String:
extern const u2 javaLangStringClassIndex;

// The classId of java.lang.Class:
extern const u2 javaLangClassClassIndex;

// The collection of constant pools - the size of this is == 'numberOfAllClassInstanceInfo':
extern const constantPool const allConstantPools[];

extern const u2 numberOfAllMethodsInAllClasses;
extern const methodInClass const allMethodsInAllClasses[];

// Native jump table:
extern const nativeJumpTableEntry const nativeJumpTable[];
extern const u2 nativeJumpTableSize;

extern const u2 numberOfAllFieldReferences;
extern const memberReference const allFieldReferences[];

extern const u2 numberOfAllFields;
extern const fieldInClass const allFields[];

extern const u2 numberOfAllIntegerConstantReferences;
extern const integerConstantReference const allIntegerConstantReferences[];

extern const u2 numberOfAllFloatConstantReferences;
extern const floatConstantReference const allFloatConstantReferences[];

extern const u2 numberOfAllDoubleConstantReferences;
extern const doubleConstantReference const allDoubleConstantReferences[];

extern const u2 numberOfAllStringConstantReferences;
extern const stringConstantReference const allStringConstantReferences[];

extern const codeIndex startAddress;
extern const u2 startClassIndex;

extern const u2 const implementedInterfaces[];

// If silent, then VM shall be silent (no debug info) when executing:
extern BOOL silent;

// The array of java.lang.Class:
extern jobject javaLangClassArray;

// The Exception handler array:
extern const exceptionHandler const allExceptionHandersInAllClasses[];

// The number of elements in theException Handler Array:
extern const u2 numberOfAllExceptionHandersInAllClasses;

// The build-in / expected dependencies:
extern const BuildinClassDependency_s const AllBuildinClassDependencies[];
extern const u2 numberOfAllBuildinClassDependencies;
extern const BuildinMemberDependency_s const AllBuildinMemberDependencies[];
extern const u2 numberOfAllBuildinMemberDependencies;

/**
 * This method generates the Class[] to be used for Object.getClass(), Class.class (named javaLangClassArray):
 */
extern void generateJavaLangClassInstances();

/**
 * This method returns the Class - instance corresponding to the supplied classId.
 * \param requestedClassId the id to resolve
 * \return The resolved jclass
 */
extern jclass getJavaLangClass(u2 requestedClassId);

/**
 * This method invokes a native method identified by the parameter
 * \param nativeIndex The index (+1) into the native jump table
 */
void invokeNativeMethod(u2 nativeIndex);

/**
 * This method looks up an instance method of a class. This will recurse up through super classes
 * until a match is found.
 * \param index The index identifying the static method within constant pool identified by classIndex
 * \return A method definition pointer. Points into the autogenerated table
 */
const methodInClass* getVirtualMethodEntry(u2 index);

/**
 * This method looks up a static method of a class.
 *
 * \param classId The referencing class (typically  == context.classIndex)
 * \param cp_index The constant pool index identifying the static method within constant pool
 *  identified by context.classIndex
 * \return A method definition pointer. Points into the autogenerated table
 */
const methodInClass* getStaticMethodEntry(u2 classId, u2 cp_index);
/**
 * This method looks up a static method of a class.
 *
 * \param index The index identifying the static method within constant pool identified by classIndex
 * \param localVariableCount The number of local variables, including the arguments
 * \param argumentCount The number of arguments
 * \param nativeIndex The native index. If > 0, the method is a native method.
 */
void getStaticMethodEntry_deprecated(u2 index, codeIndex* destAddr, u2* dstClassIndex,
		u1* localVariableCount, u1* argumentCount, u2* nativeIndex);

/**
 * This method looks up a static field of a class
 * \param index The class index
 * \param address The static address of the field
 * \param size The size of the field
 */
void getStaticFieldEntry(u2 index, u2* address, u1* size);

/**
 * This method looks up an instance field of a class
 * \param index The class index
 * \param address The instance address of the field (relative to 'this')
 * \param size The size of the field
 */
void getInstanceFieldEntry(u2 index, u2* address, u1* size);

/**
 * This method looks up a constant within current class' constant pool
 */
void getConstant(u2 constantPoolIndex, constantDef* constant);

/**
 * This method looks up the size of an instance of a class
 */
void getClassSize(u2 classId, u2* size);

/**
 * This method looks up a class reference in list of class references
 */
void getClassReference(u2 constantPoolIndex, u2* classId);

/**
 * This method looks up an array type and retrieves the array info
 * \param classId The class id for the array type class
 * \param size The size in bytes of a single array element
 * \param elementClassId The class id of a single array element
 */
void getArrayInfo(u2 classId, u2* elementClassId, size_t* size);

/**
 * This method returns the class id of the first class having the indicated type.
 * \param type The class type to search for
 * \return The class id of the first class having the indicated type.
 */
u2 getClassIdForClassType(CLASS_TYPE type);

/**
 * This method returns true if the classId refers to an array of objectrefs
 * \param classId The id to lookup
 * \return true if and only if the classId refers to an array of objectrefs
 */
BOOL isObjectArray(u2 classId);

/**
 * This method returns true if the classId refers to an array of primitive values
 * \param classId The id to lookup
 * \return true if and only if the classId refers to an array of primitive values
 */
BOOL isPrimitiveValueArray(u2 classId);

/**
 * This method looks up the type (interface/class) of a class
 * \param classId the class id to lookup with
 * \return The resulting type. If not match, the method will exit the vm.
 */
CLASS_TYPE getClassType(u2 classId);

/**
 * This function converts a CLASS_TYPE to a readable string
 * \param ctyp The value to convert
 * \return a non-null char*
 */
char* getClassTypeString(CLASS_TYPE ctyp);

/**
 * This function tests if class S implements interface T
 * \param classId_S The class id to test
 * \param classId_T The interface to test for implementation of
 * \return true, if implementing; false otherwise
 */
BOOL is_S_implementing_T(u2 classId_S, u2 classId_T);

/**
 * This function tests if class S is sub class of class T
 * \param classId_S The (sub-) class id to test
 * \param classId_T The class id to test against
 * \return true, if sub classing; false otherwise
 */
BOOL is_S_SubClassing_T(u2 classId_S, u2 classId_T);

/**
 * This method looks up the array element classId for a given class (which is assume to be an array; this is
 * the callers responsibility)
 * \param classId The id of the class to lookup
 * \return The classId of the elements in the array.
 */
u2 getArrayElementClassId(u2 classId);

/**
 * This method converts an array type into a class type
 * \param atyp The ARRAY_TYPE to convert
 * \return The converted CLASS_TYPE.
 */
CLASS_TYPE convertArrayType(ARRAY_TYPE atyp);

/**
 * This returns the class id of the array class where the element is elementClassId.
 * \param elementClassid Identifies the element Class Id of the class to lookup
 * \return the ClassId of the array-of-element-class
 */
u2 getArrayClassIdForElementClassId(u2 elementClassId);

/**
 * The total number of defined instructions
 */
extern const size_t numberOfDefinedInstructions;

/**
 * This method looks up a static method entry in the methods table.
 *
 * \param referencedClassId The referenced class
 * \param linkId The linkId for the method
 */
const methodInClass* getStaticMethodEntryByLinkId(u2 referencedClassId, u2 linkId);

/**
 * This method looks up an instance method entry in the methods table.
 *
 * \param referencedClass The referenced class.
 * \param linkId The linkId for the method
 */
const methodInClass* getVirtualMethodEntryByLinkId(jobject jref, u2 linkId);

/**
 * This method will open a new frame and call the method indicated in the parameters.
 *
 * \param mic A pointer to a method definition to be invoked.
 * \param returnFromVM if TRUE: When a frame is popped (using pop_frame) and this is true,
 * the execute() - function will return; if FALSE: No return will happen from the execute()
 * function. Shall be set to TRUE when a native function is calling a java method.
 */
void invokeCommon(const methodInClass* mic, BOOL returnFromVM);

/**
 * This method returns the classId of the build-in dependency identified by libref
 * \param libref Identifies the dependency
 * \return The corresponding class Id or no return at all, if no match
 */
u2 getClassBuildinDependecy(BuildinDependency_e libref);

/**
 * This method returns the linkId of the build-in dependency identified by libref
 * \param libref Identifies the dependency
 * \return The corresponding linkId or no return at all, if no match
 */
u2 getMemberBuildinDependecy(BuildinDependency_e libref);


#endif /* CONSTANTPOOL_H_ */
