#ifndef _OLIO_STRING_H_
#define _OLIO_STRING_H_

#include <string.h>

#include "array.h"

typedef olio_array olio_string;

#define OLIO_STRING_INIT(name) \
	olio_string_init(&name, sizeof(name));
#define OLIO_STRING_STACK(name, n) \
	char name##_olio_string_stack[sizeof(olio_string) + (n) * sizeof(char)]; \
	olio_string * name = olio_string_init(name##_olio_string_stack, \
		sizeof(olio_string) + (n) * sizeof(char));
#define OLIO_STRING_ALLOC(name, n) \
	name = olio_string_init(NULL, (n) * sizeof(char));

static inline olio_string * olio_string_init(void * buffer, uint16_t length)
{
	olio_string * s = (olio_string *) olio_array_init(sizeof(char),
		buffer, length);
	if (s == NULL) return NULL;
	s->base.length = 1;
	s->base.data[0] = 0;
	return s;
}

static inline void olio_string_free(olio_string * s)
{
	olio_array_free(s);
}

static inline olio_string_clear(olio_string * s)
{
	s->base.length = 1;
	s->base.data[0] = 0;
}

static inline const char * olio_string_contents(const olio_string * s)
{
	return (const char *) olio_array_contents(s);
}

static inline uint16_t olio_string_length(const olio_string * s)
{
	return olio_array_length(s) - 1;
}

static inline uint16_t olio_string_datasize(const olio_string * s)
{
	return olio_array_datasize(s);
}

static inline int olio_string_insert(olio_string * s,
	uint16_t position, const char * data, uint16_t length)
{
	if (position > s->base.length - 1)
		position = s->base.length - 1;
	return olio_array_insert_unbounded(s, position, data, length);
}

static inline void olio_string_remove(olio_string * s,
	uint16_t position, uint16_t length)
{
	if (position <= s->base.length - 1) {
		if (position + length > s->base.length - 1)
			length = s->base.length - 1 - position;
		olio_array_remove_unbounded(s, position, length);
	}
}

static inline int olio_string_append(olio_string * s, 
	const char * data, uint16_t length)
{
	return olio_array_insert_unbounded(s, s->base.length - 1, data, length);
}

static inline int olio_string_append_char(olio_string * s, char data)
{
	return olio_array_insert_unbounded(s, s->base.length - 1, &data, 1);
}

static inline void olio_string_prepend(olio_string * s,
	const char * data, uint16_t length)
{
	olio_array_prepend(s, data, length);
}

static inline int olio_string_duplicate(olio_string * s, const olio_string * d)
{
  olio_string_clear(s);
  return olio_string_append(s, olio_string_contents(d),
			    olio_string_length(d));
}

static inline int olio_string_copy(olio_string * s, const char * d,
				   uint16_t length)
{
  olio_string_clear(s);
  return olio_string_append(s, d, length);
}

static inline int olio_string_copy_cstring(olio_string * s, const char * d)
{
  olio_string_copy(s, d, strlen(d));
}

#endif /* _OLIO_STRING_H_ */
