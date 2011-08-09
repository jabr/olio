#ifndef _OLIO_RANDOM_H_
#define _OLIO_RANDOM_H_

#include <inttypes.h>

typedef struct _olio_random {
	uint32_t seed;
	uint32_t state[627];
} olio_random;

#ifdef __cplusplus
extern "C" {
#endif

void olio_random_set_seed(olio_random *, uint32_t seed);
uint32_t olio_random_generate_seed(olio_random *);
uint32_t olio_random_integer(olio_random *); /* [0x0,0xffffffff] */

#ifdef __cplusplus
} /* extern C */
#endif

static inline uint32_t olio_random_get_seed(olio_random * r)
{ return r->seed; }

static inline double olio_random_real_ii(olio_random * r) /* [0,1] */
{ return (double) olio_random_integer(r) / 4294967295.0; }
static inline double olio_random_real_ie(olio_random * r) /* [0,1) */
{ return (double) olio_random_integer(r) / 4294967296.0; }
static inline double olio_random_real_ei(olio_random * r) /* (0,1] */
{ return ((double) olio_random_integer(r) + 1.0) / 4294967296.0; }
static inline double olio_random_real_ee(olio_random * r) /* (0,1) */
{ return ((double) olio_random_integer(r) + 0.5) / 4294967296.0; }

#endif /* _OLIO_RANDOM_H_ */
