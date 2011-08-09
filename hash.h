#ifndef _OLIO_HASH_H_
#define _OLIO_HASH_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t olio_hash_value(const void * key, uint16_t len, uint32_t seed);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _OLIO_HASH_H_ */
