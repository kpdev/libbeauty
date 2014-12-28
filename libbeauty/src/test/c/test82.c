/* A very simple function to test memory stores. */

#include <stdint.h>

int8_t mem1 = 8;

int32_t test82(int8_t c) {
	int8_t a;
	int8_t b;
	int32_t d;

	a = mem1;
	b = c ^ a;
	d = b;
	return d;
}

