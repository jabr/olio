#include "varint.h"

uint32_t olio_varint_get(const uint8_t * v, uint8_t * bytes)
{
    uint32_t value = 0;
    uint8_t i = 0;
    do {
	value <<= 7;
	value |= (v[i] & 0x7f);
    } while (v[i++] & 0x80);

    *bytes = i;
    return value;
}

uint8_t olio_varint_set(uint32_t v, uint8_t * storage)
{
    uint8_t d = 0;
    {
	uint32_t x = v >> 7;
	while (x) { d++; x >>= 7; }
    }
    {
	uint8_t i = 0;
	do {
	    uint8_t s = v & 0x7f;
	    v >>= 7;
	    storage[d-i] = s | 0x80;
	    i++;
	} while (v > 0);
	storage[d] &= 0x7f;
    }
    return (d + 1);
}
