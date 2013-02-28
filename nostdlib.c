/*
 * mystdlib.c
 *
 *  Created on: Jan 14, 2012
 *      Author: hammer
 */

#include <string.h>
#include "config.h"
#include "types.h"

// This c-file shall not be used when running native GCC (in contrast to cross-compiling GCC); when
// using native GCC it's up to GCC to link in the correct low level functions.

#ifndef OMIT_STDLIB
void *memcpy(void *s1, const void * s2, size_t n) {
	void *org = s1;
	while (n-- > 0) {
		*((unsigned char *) s1++) = *((unsigned char *) s2++);
	}

	return org;
}

void *memset(void *s1, int c, size_t len) {
	////	consout("%s:%d c = %d, len = %d\n", __FILE__, __LINE__, c, len);
	//	size_t i;
	//	for (i = 0; i < len; i++) {
	//		if ((i & 0xff) == 0xff) {
	//			thinj_putchar('*');
	//		}
	////		consout("%s:%d i = %d\n", __FILE__, __LINE__, i);
	//		char*cp = s1 + i;
	//		*cp = (char)c;
	//	}
	//	thinj_putchar('\n');
	////	consout("%s:%d c = %d, len = %d\n", __FILE__, __LINE__, c, len);
	void *org = s1;
	while (len-- > 0) {
		*((unsigned char*) s1++) = (unsigned char) c;
	}

	return org;
}

size_t strlen(const char *s) {
	size_t len = 0;
	if (s != NULL) {
		while (*s++) {
			len++;
		}
	}

	return len;
}

/**
 * This method is implicitly called from va_arg macro.
 */
void abort(void) {
	jvmexit(1);
}
/*
 /home/hammer/workspace/bluevm/frame.c:50: undefined reference to `memcpy'
 frame.o(.text+0x31c):/home/hammer/workspace/bluevm/frame.c:79: undefined reference to `memcpy'
 frame.o(.text+0x3a4): In function `resetVM':
 /home/hammer/workspace/bluevm/frame.c:171: undefined reference to `memset'
 heap.o(.text+0x178): In function `validateList':
 /home/hammer/workspace/bluevm/heap.c:72: undefined reference to `exit'
 heap.o(.text+0x720): In function `_heapAlloc':
 /home/hammer/workspace/bluevm/heap.c:249: undefined reference to `memset'
 heap.o(.text+0xee4): In function `heapAllocPrimitiveTypeArray':
 /home/hammer/workspace/bluevm/heap.c:425: undefined reference to `memset'
 heap.o(.text+0x1010): In function `heapAllocObjectArray':
 /home/hammer/workspace/bluevm/heap.c:451: undefined reference to `memset'
 */

//
// From http://www.helenos.org/doc/refman/uspace-mips32/group__softint.html:
//
// static unsigned int 	      divandmod32 (unsigned int a, unsigned int b, unsigned int *remainder)
// static unsigned long long  divandmod64 (unsigned long long a, unsigned long long b, unsigned long long *remainder)
// int __divsi3(int a, int b)
// long long __divdi3 (long long a, long long b)
// unsigned int 	__udivsi3 (unsigned int a, unsigned int b)
// unsigned long long 	__udivdi3 (unsigned long long a, unsigned long long b)
// int __modsi3(int a, int b)
// long long __moddi3 (long long a, long long b)
// unsigned int	__umodsi3 (unsigned int a, unsigned int b)
// unsigned long long 	__umoddi3 (unsigned long long a, unsigned long long b)
// unsigned long long 	__udivmoddi3 (unsigned long long a, unsigned long long b, unsigned long long *c)

unsigned int __udivsi3(register unsigned int a, register unsigned int b) {
	char count = 1;

	if (b == 0) {
		// Division by zero:
		jvmexit(1);
	}

	while ((b < a) && !(b & 0x80000000)) {
		b <<= 1;
		count++;
	}

	register int res = 0;
	while (count-- > 0) {
		res <<= 1;
		if (a >= b) {
			a -= b;
			res++;
		}
		b >>= 1;
	}

	return res;
}

int __divsi3(register int a, register int b) {
	unsigned char negative = FALSE;
	if (a < 0) {
		a = -a;
		negative = !negative;
	}
	if (b < 0) {
		b = -b;
		negative = !negative;
	}

	unsigned int res = __udivsi3(a, b);
	return negative ? -res : res;
}

int __modsi3(register int a, register int n) {
	//	return 7;
	// From wiki:
	// (...)Use truncated division where the quotient is defined by truncation q = trunc(a/n),
	// in other words it is the first integer in the direction of 0 from the exact rational
	// quotient, and the remainder by r = a − n x q.
	unsigned char negative = FALSE;
	if (a < 0) {
		a = -a;
		negative = TRUE;
	}
	if (n < 0) {
		n = -n;
	}
	int modulus = a - n * __divsi3(a, n);
	if (negative) {
		modulus = -modulus;
	}
	return modulus;
}

////////////////////////////////////////////////////////////////////
// 64 bit math
////////////////////////////////////////////////////////////////////
typedef union __u8u4 {
	struct {
		u8 lo;
	} asU8;
	struct {
		u4 lo;
		u4 hi;
	} asU4;
} u8u4;

s8 __ashldi3(s8 u, int shift) {
	// Arith shift left
	u8u4 x;
	x.asU8.lo = u;

	shift = shift & 0x3f;
	while (shift != 0) {
		x.asU4.hi <<= 1;
		if ((x.asU4.lo & 0x80000000) != 0) {
			x.asU4.hi |= 1;
		}
		x.asU4.lo <<= 1;
		shift--;
	}

	return x.asU8.lo;
}

s8 __ashrdi3(s8 u, int shift) {
	// Arithmetic shift right
	u8u4 x;
	x.asU8.lo = u;

	shift = shift & 0x3f;
	u4 sign = x.asU4.hi & 0x80000000;
	while (shift != 0) {
		x.asU4.lo >>= 1;
		if ((x.asU4.hi & 1) != 0) {
			x.asU4.lo |= 0x80000000;
		}
		x.asU4.hi = sign | (x.asU4.hi >> 1);
		shift--;
	}

	return x.asU8.lo;
}

s8 __lshrdi3(s8 u, int shift) {
	// Logic shift right
	u8u4 x;
	x.asU8.lo = u;

	shift = shift & 0x3f;
	while (shift != 0) {
		x.asU4.lo >>= 1;
		if ((x.asU4.hi & 1) != 0) {
			x.asU4.lo |= 0x80000000;
		}
		x.asU4.hi = (x.asU4.hi >> 1) & 0x7fffffff;
		shift--;
	}

	return x.asU8.lo;
}

u8 __udivdi3(u8 a, u8 b) {
	char count = 1;

	if (b == 0) {
		// Division by zero:
		jvmexit(1);
	}

	while ((b < a) && !(b & 0x8000000000000000)) {
		b <<= 1;
		count++;
	}

	u8 res = 0;
	while (count-- > 0) {
		res <<= 1;
		if (a >= b) {
			a -= b;
			res++;
		}
		b >>= 1;
	}

	return res;
}

s8 __divdi3(s8 a, s8 b) {
	unsigned char negative = FALSE;
	if (a < 0) {
		a = -a;
		negative = !negative;
	}
	if (b < 0) {
		b = -b;
		negative = !negative;
	}

	u8 res = __udivdi3(a, b);
	return negative ? -res : res;
}

u8 __moddi3(u8 a, u8 n) {
	//	return 7;
	// From wiki:
	// (...)Use truncated division where the quotient is defined by truncation q = trunc(a/n),
	// in other words it is the first integer in the direction of 0 from the exact rational
	// quotient, and the remainder by r = a − n x q.
	unsigned char negative = FALSE;
	if (a < 0) {
		a = -a;
		negative = TRUE;
	}
	if (n < 0) {
		n = -n;
	}
	s8 modulus = a - n * __divdi3(a, n);
	if (negative) {
		modulus = -modulus;
	}
	return modulus;
}

#endif

#if JUST_TESTING
#include "types.h"

unsigned int udivsi3(register unsigned int a, register unsigned int b) {
	char count = 1;

	if (b == 0) {
		// Division by zero:
		jvmexit(1);
	}

	while ((b < a) && !(b & 0x80000000)) {
		b <<= 1;
		count++;
	}

	register int res = 0;
	while (count-- > 0) {
		res <<= 1;
		if (a >= b) {
			a -= b;
			res++;
		}
		b >>= 1;
	}

	return res;
}

int divsi3(register int a, register int b) {
	unsigned char negative = FALSE;
	if (a < 0) {
		a = -a;
		negative = !negative;
	}
	if (b < 0) {
		b = -b;
		negative = !negative;
	}

	unsigned int res = udivsi3(a, b);
	return negative ? -res : res;
}

int modsi3(register int a, register int n) {
	//	return 7;
	// From wiki:
	// (...)Use truncated division where the quotient is defined by truncation q = trunc(a/n),
	// in other words it is the first integer in the direction of 0 from the exact rational
	// quotient, and the remainder by r = a − n x q.
	unsigned char negative = FALSE;
	if (a < 0) {
		a = -a;
		negative = TRUE;
	}
	if (n < 0) {
		n = -n;
	}
	int modulus = a - n * divsi3(a, n);
	if (negative) {
		modulus = -modulus;
	}
	return modulus;
}

void mt(int a, int b, int expected) {
	//printf("%d, %d\n", modsi3(a, b), a % b);
	if (modsi3(a, b) != expected) {
		printf("fejle: %d %% %d == %d, expected %d\n", a, b, modsi3(a, b), expected);
	}
}

void mmtt(int product, int divisor) {
	if (divisor == 1 || divisor == -1) {
		mt((product + divisor - 1), divisor, 0);
	} else if (product + divisor - 1 < 0) {
		mt((product + divisor - 1), divisor, -1);
	} else {
		if (divisor - 1 < 0) {
			mt((product + divisor - 1), divisor, -divisor - 1);
		} else {
			mt((product + divisor - 1), divisor, divisor - 1);
		}
	}
}

void modtest() {
	int ui, uj;

	for (ui = -1000; ui < 1000; ui += 9) {
		for (uj = -1000; uj < 1000; uj += 7) {
			if (ui != 0 && uj != 0) {
				int prod = ui * uj;
				mt(prod, ui, 0);
				mt(prod, uj, 0);
				//				Regression.verify(prod % uj == 0);
				//
				//				System.out.println("moda: " + (prod + ui - 1) % ui);
				//				System.out.println("modb: " + (prod + uj - 1) % uj);
				//				Regression.verify((prod + ui - 1) % ui == ui - 1);
				//				Regression.verify((prod + uj - 1) % uj == uj - 1);

				//				if ((++count % 10000) == 0) {
				//					System.out.println("count = " + count);
				//				}
				mmtt(prod, ui);
				mmtt(prod, uj);
			}
		}
	}

	consout("fortegn %d\n", -6);
}
#endif // JUST_TESTING
