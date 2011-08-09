#ifndef _OLIO_ARRAY_H_
#define _OLIO_ARRAY_H_

#include <inttypes.h>

/* the string code (string.h) directly access this structure,
 * so changes here need to be reflected in the string code */
typedef struct _olio_array_base {
	uint16_t length;
	uint16_t allocated;
	uint8_t * data;
	uint16_t stack_allocated;
	uint16_t size;
	uint8_t flags;
} olio_array_base;

typedef struct _olio_array {
	olio_array_base base;
	/* we separate the base from the stack array to ensure
	 * proper alignment of the stack for any data type. */
	uint8_t stack[1];
} olio_array;

#define OLIO_ARRAY_INIT(name, t) \
	olio_array_init(sizeof(t), &name, sizeof(name));
#define OLIO_ARRAY_STACK(name, t, n) \
	char name##_olio_array_stack[sizeof(olio_array) + (n) * sizeof(t)]; \
	olio_array * name = olio_array_init(sizeof(t), name##_olio_array_stack, \
		sizeof(olio_array) + (n) * sizeof(t));
#define OLIO_ARRAY_ALLOC(name, t, n) \
	name = olio_array_init(sizeof(t), NULL, (n) * sizeof(t));

#ifdef __cplusplus
extern "C" {
#endif

olio_array * olio_array_init(uint16_t size, void * buffer, uint16_t length);
void olio_array_free(olio_array * a);
int olio_array_insert_unbounded(olio_array *, 
	uint16_t postion, const void * data, uint16_t length);
void olio_array_remove_unbounded(olio_array *, 
	uint16_t position, uint16_t length);

#ifdef __cplusplus
} /* extern C */
#endif

static inline void olio_array_clear(olio_array * a)
{
	a->base.length = 0;
}

static inline const void * olio_array_contents(const olio_array * a)
{
	return a->base.data;
}

static inline uint16_t olio_array_length(const olio_array * a)
{
	return a->base.length;
}

static inline uint16_t olio_array_datasize(const olio_array * a)
{
	return a->base.size;
}

static inline int olio_array_insert(olio_array * a,
	uint16_t position, const void * data, uint16_t length)
{
	if (position > a->base.length) position = a->base.length;
	return olio_array_insert_unbounded(a, position, data, length);
}

static inline void olio_array_remove(olio_array * a,
	uint16_t position, uint16_t length)
{
	if (position <= a->base.length) {
		if (position + length > a->base.length)
			length = a->base.length - position;
		olio_array_remove_unbounded(a, position, length);
	}
}

static inline int olio_array_append(olio_array * a, 
	const void * data, uint16_t length)
{
	return olio_array_insert_unbounded(a, a->base.length, data, length);
}

static inline int olio_array_prepend(olio_array * a, 
	const void * data, uint16_t length)
{
	return olio_array_insert_unbounded(a, 0, data, length);
}

#endif /* _OLIO_ARRAY_H_ */
