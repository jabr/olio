#ifndef _OLIO_VARINT_H_
#define _OLIO_VARINT_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

    uint32_t olio_varint_get(const uint8_t * v, uint8_t * bytes);
    uint8_t olio_varint_set(uint32_t v, uint8_t * storage);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _OLIO_VARINT_H_ */

