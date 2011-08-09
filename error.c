#include <stdio.h>

#include "error.h"

void olio_error_display(olio_error * e)
{
	fprintf(stderr, "code: %lu subcode: %lu\n" "message: %s\n"
		"submessage: %s\n",
		e->code, e->subcode, 
		(e->message != NULL) ? e->message : "(none)",
		(e->submessage != NULL) ? e->submessage : "(none)");
}

void olio_error_warning_default(olio_error * e, const char * msg, va_list arg)
{
	fprintf(stderr, "Warning: ");
	vfprintf(stderr, msg, arg);
	fprintf(stderr, "\n");
}
