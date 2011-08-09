#include <stdlib.h>
#include <string.h>

#include "array.h"

static int _expand_array(olio_array * a, uint16_t length)
{
	int copy = 0;
	a->base.allocated = (length + 0x1ff) & 0xfe00;
	if (a->base.data == a->stack) {
		copy = 1;
		a->base.data = NULL;
	}
	a->base.data = (uint8_t *) realloc(a->base.data, 
		a->base.allocated * a->base.size);
	if (a->base.data == NULL) return -1;
	if (copy) memmove(a->base.data, a->stack, a->base.length * a->base.size);
	return 0;
}

olio_array * olio_array_init(uint16_t size, 
	void * buffer, uint16_t length)
{
	olio_array * a;
	uint16_t l;
	if (buffer == NULL) {
		l = sizeof(olio_array) + length;
		a = (olio_array *) malloc(l);
		if (a == NULL) return NULL;
		a->base.flags = 0x01;
	} else {
		l = length;
		if (l < sizeof(olio_array)) return NULL;
		a = (olio_array *) buffer;
		a->base.flags = 0x00;
	}

	a->base.allocated = a->base.stack_allocated = 
		(l - ((void *) &a->stack[0] - (void *) a)) / size;
	a->base.length = 0;
	a->base.size = size;
	a->base.data = a->stack;
	return a;
}

void olio_array_free(olio_array * a)
{
	if (a->base.data != a->stack && a->base.data != NULL) free(a->base.data);
	if (a->base.flags & 0x01) free(a);
}

int olio_array_insert_unbounded(olio_array * a, uint16_t position, 
	const void * data, uint16_t length)
{
	if (a->base.length + length > a->base.allocated) {
		if (_expand_array(a, a->base.length + length))
			return -1;
	}

	memmove(a->base.data + (length + position) * a->base.size,
		a->base.data + position * a->base.size,
		(a->base.length - position) * a->base.size);
	memmove(a->base.data + position * a->base.size, data, 
		length * a->base.size);
	a->base.length += length;
	return 0;
}

void olio_array_remove_unbounded(olio_array * a, 
	uint16_t position, uint16_t length)
{
	memmove(a->base.data + (a->base.length + position) * a->base.size,
		a->base.data + (a->base.length + position + length) * a->base.size,
		length * a->base.size);

	a->base.length -= length;
}
