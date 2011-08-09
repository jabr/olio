#ifndef _OLIO_FOP_H_
#define _OLIO_FOP_H_

#include "Judy.h"

#define OLIO_FOB_TYPE_HASH    0
#define OLIO_FOB_TYPE_LIST    1
#define OLIO_FOB_TYPE_STRING  2
#define OLIO_FOB_TYPE_INT32   3
#define OLIO_FOB_TYPE_INT64   4
#define OLIO_FOB_TYPE_FLOAT   5
#define OLIO_FOB_TYPE_DOUBLE  6
#define OLIO_FOB_TYPE_OTHER   7

typedef struct _olio_fob {
	uint16_t size;
	uint16_t first_free;
	Pvoid_t array;
	uint8_t type;
	uint8_t * data;
} olio_fob;

#endif /* _OLIO_FOB_H_ */
