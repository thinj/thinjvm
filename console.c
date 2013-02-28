/*
 * console.c
 *
 *  Created on: Nov 10, 2010
 *      Author: hammer
 */

#include <stdlib.h>
#include <stdarg.h>

#include "frame.h"
#include "console.h"
#include "types.h"
#include "debugger.h"


/**
 * This method prints a decimal long (8 bytes)
 */
static void common_print_d(char prefix, size_t size, s8 value, s8 limit, int remaining_digits) {
	BOOL in_number = FALSE;
	if (value < 0) {
		thinj_putchar('-');
		value = -value;
		if (size) {
			size--;
		}
	}

	while (limit > 0) {
		if (limit == 1) {
			in_number = TRUE;
		}
		if (value >= limit) {
			thinj_putchar(value / limit + '0');
			value = value % limit;
			in_number = TRUE;
			if (size) {
				size--;
			}
		} else if (in_number) {
			thinj_putchar(value / limit + '0');
			if (size) {
				size--;
			}
		} else if (size >= remaining_digits) {
			thinj_putchar(prefix);
			size--;
		}
		remaining_digits--;
		limit /= 10;
	}

}


/**
 * This method prints a decimal s8
 */
static void print_ld(char prefix, size_t size, s8 value) {
	// 0xffffffffffffffff = 18446744073709551615
	// 0x7fffffffffffffff = 9223 3720 3685 4775 807
	common_print_d(prefix, size, (s8) value, 1000000000000000000, 19);
}
/**
 * This method prints a decimal int
 */
static void print_d(char prefix, size_t size, int value) {
//	int limit = 1000000000; // 32 bit int
//	int remaining_digits = 10;
	common_print_d(prefix, size, (s8) value, 1000000000, 10);
	/*
	BOOL in_number = FALSE;
	if (value < 0) {
		thinj_putchar('-');
		value = -value;
		if (size) {
			size--;
		}
	}
	while (limit > 0) {
		if (limit == 1) {
			in_number = TRUE;
		}
		if (value >= limit) {
			thinj_putchar(value / limit + '0');
			value = value % limit;
			in_number = TRUE;
			if (size) {
				size--;
			}
		} else if (in_number) {
			thinj_putchar(value / limit + '0');
			if (size) {
				size--;
			}
		} else if (size >= remaining_digits) {
			thinj_putchar(prefix);
			size--;
		}
		remaining_digits--;
		limit /= 10;
	}
	*/
}

static const char* HEX_CHAR = "0123456789abcdef";
static void print_x(char prefix, size_t size, unsigned int value) {
	int remaining_bits = 32;
	BOOL in_number = FALSE;
	while (remaining_bits) {
		remaining_bits -= 4;
		if (value & (0xf << remaining_bits)) {
			in_number = TRUE;
			char ch = HEX_CHAR[(value >> remaining_bits) & 0x0f];
			thinj_putchar(ch);
			value -= value & (0xf << remaining_bits);
			if (size) {
				size--;
			}
		} else if (in_number) {
			char ch = HEX_CHAR[(value >> remaining_bits) & 0x0f];
			thinj_putchar(ch);
			value -= value & (0xf << remaining_bits);
			if (size) {
				size--;
			}
		} else if (size > remaining_bits / 4) {
			thinj_putchar(prefix);
			size--;
		}
	}
}

static void print_cp(char prefix, size_t size, char* cp) {
	char *tmp = cp;
	size_t length = 0;
	while (*tmp++) {
		length++;
	}

	while (size-- > length) {
		thinj_putchar(prefix);
	}

	while (*cp) {
		thinj_putchar(*cp);
		cp++;
	}
}

typedef enum {
	// Searching for '%':
	SEEK,
	// Found '%', what's next (prefix, size or type):
	BEGIN_FORMAT,
	// Parsing the size:
	SIZE,
	// Parsing the type:
	TYPE
} STATE_ENUM;

void consout(char * format, ...) {
	va_list args;
	va_start (args, format);

	char prefix = '\0';
	STATE_ENUM state = SEEK;
	size_t size = 0;
	while (*format) {
		switch (state) {
		case SEEK:
			if (*format == '%') {
				state = BEGIN_FORMAT;
			} else {
				thinj_putchar(*format);
			}
			format++;
			break;
		case BEGIN_FORMAT:
			if (*format == '%') {
				thinj_putchar('%');
				state = SEEK;
				format++;
			} else {
				if (*format == '0') {
					prefix = '0';
					format++;
					state = SIZE;
				} else if ('1' <= *format && *format <= '9') {
					prefix = ' ';
					state = SIZE;
				} else {
					state = TYPE;
				}
			}
			break;
		case SIZE:
			if ('0' <= *format && *format <= '9') {
				size *= 10;
				size += *format - '0';
				format++;
			} else {
				state = TYPE;
			}
			break;
		case TYPE:
			switch (*format) {
			case 'l': {
				format++;
				switch (*format) {
				case 'd': {
					s8 value = va_arg(args, s8);
					print_ld(prefix, size, value);
					break;
				}
				}
				break;
			}
			case 'd': {
				int value = va_arg(args, int);
				print_d(prefix, size, value);
				break;
			}
			case 'p': {
				print_cp(prefix, size, "0x");
				// Platform dependent:
				unsigned int value = (unsigned int) va_arg(args, void*);
				//unsigned int value = (unsigned int) va_arg(args, unsigned int);
				print_x(' ', 0, value);
				break;
			}
			case 'x': {
				unsigned int value = (unsigned int) va_arg(args, unsigned int);
				print_x(prefix, size, value);
				break;
			}
			case 's': {
				char* cp = va_arg(args, char*);
				if (!cp) {
					print_cp(prefix, size, "(null)");
				} else {
					print_cp(prefix, size, cp);
				}
				break;
			}
			case 'c': {
				// One character
				char cp[2];
				/*
				 * WARNING: The 2nd argument shall be 'int' not 'char' - otherwise the program terminates
				 * with an 'Illegal Instruction' signal
				 */
				cp[0] = va_arg(args, int);
				cp[1] = '\0';
				print_cp(prefix, size, cp);
				break;
			}
			default:
				thinj_putchar('?');
				break;
			}
			state = SEEK;
			format++;
			size = 0;
			prefix = '\0';
			break;
		}
	}

	va_end (args);
}

