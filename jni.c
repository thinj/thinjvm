/*
 * jni.c
 *
 *  Created on: Feb 20, 2012
 *      Author: hammer
 */

#include "jni.h"
#include "trace.h"
#include "instructions.h"

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
