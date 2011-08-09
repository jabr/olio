#ifndef _OLIO_DEBUG_H_
#define _OLIO_DEBUG_H_

#if defined(OLIO_DEBUG_ON) || !defined(OLIO_DEBUG_OFF)
#include <stdio.h>
#define TRACE(x) { fprintf(stderr, x); fflush(stderr); }
#else
#define TRACE(x)
#endif

#endif /* _OLIO_DEBUG_H_ */
