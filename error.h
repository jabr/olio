#ifndef _OLIO_ERROR_H_
#define _OLIO_ERROR_H_

#include <stdarg.h>
#include <inttypes.h>

struct _olio_error;

typedef void(*olio_error_warning_callback)
(struct _olio_error *, const char *, va_list);

typedef struct _olio_error {
	uint32_t module;
	uint32_t code;
	uint32_t subcode;
	const char * message;
	const char * submessage;
	olio_error_warning_callback handle_warning;
} olio_error;

#ifdef __cplusplus
extern "C" {
#endif

void olio_error_warning_default(olio_error *, const char *, va_list);
void olio_error_display(olio_error *);

#ifdef __cplusplus
} /* extern C */
#endif

static inline void olio_error_warning(olio_error * e, const char * msg, va_list arg)
{
	if (e->handle_warning != NULL)
		e->handle_warning(e, msg, arg);
}

static inline void olio_error_init(olio_error * e, 
				   olio_error_warning_callback c)
{
	e->code = e->subcode = 0;
	e->message = NULL;
	e->handle_warning = c;
}

static inline int olio_error_test(olio_error * e)
{
	return (e->code != 0);
}

static inline void olio_error_clear(olio_error * e)
{
	e->code = e->subcode = 0;
	e->message = e->submessage = NULL;
}

static inline void olio_error_set(olio_error * e, 
				  uint32_t code, uint32_t subcode,
				  const char * message,
				  const char * submessage)
{
	e->code = code;
	e->subcode = subcode;
	e->message = message;
	e->submessage = submessage;
}

#endif /* _OLIO_ERROR_H_ */
