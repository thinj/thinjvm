/*
 * trace.h
 *
 *  Created on: Oct 28, 2011
 *      Author: hammer
 */

#ifndef TRACE_H_
#define TRACE_H_

#ifdef USE_DEBUG
#define TRACE
#endif

#define TRACE
#ifdef TRACE
typedef struct __FRAME {
	struct __FRAME* prev;
	int line;
	char* file;
} FRAME;

extern FRAME* __fp;

// #define BEGIN consout("%s:%d\n", __FILE__, __LINE__); FRAME __f; __f.prev = __fp; __f.line = 0; __fp = &__f
#define BEGIN FRAME __f; __f.prev = __fp; __f.line = 0; __fp = &__f
#define END __fp = __f.prev
#define CALL(X)  __f.line = __LINE__; __f.file = __FILE__; X; __f.line = 0
#define DUMP_STACKTRACE(S) \
	do { \
		consout("Failed: %s\n", S);\
		FRAME* f = __fp; \
		while (f != NULL) { \
			if (f->line != 0) { \
				consout("%s: %d\n", f->file, f->line);\
			} \
			else { \
				consout("%s: %d\n", __FILE__, __LINE__);\
			} \
			f = f->prev;\
		}\
	} while(0)
#else
#define BEGIN
#define END
#define CALL(X) X
#define DUMP_STACKTRACE(S)
#endif


#endif /* TRACE_H_ */
