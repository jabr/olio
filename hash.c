#include "hash.h"

uint32_t olio_hash_value(const void *key, uint16_t len, uint32_t seed)
{
	uint32_t a, b, c;
	uint8_t * keyn = (uint8_t *) key;

#ifdef OLIO_DEBUG_HASH
	printf("hash (%08x): ", seed);
	for (a = 0; a < len; a++) {
		printf("%c", keyn[a]);
	}	
#endif

	a = b = 0x9e3779b9;
	c = seed;

#define OLIO_HASH_MIX(a,b,c) \
{	a -= b; a -= c; a ^= (c>>13); \
	b -= c; b -= a; b ^= (a<< 8); \
	c -= a; c -= b; c ^= (b>>13); \
	a -= b; a -= c; a ^= (c>>12); \
	b -= c; b -= a; b ^= (a<<16); \
	c -= a; c -= b; c ^= (b>> 5); \
	a -= b; a -= c; a ^= (c>> 3); \
	b -= c; b -= a; b ^= (a<<10); \
	c -= a; c -= b; c ^= (b>>15); }

	while (len >= 12)
	{
		a += keyn[0] + keyn[1] << 8 + keyn[2] << 16 + keyn[3] << 24;
		b += keyn[4] + keyn[5] << 8 + keyn[6] << 16 + keyn[7] << 24;
		c += keyn[8] + keyn[9] << 8 + keyn[10] << 16 + keyn[11] << 24;
		OLIO_HASH_MIX(a, b, c);
		keyn += 12; len -= 12;
	}

	c += len;
	switch (len)
	{
		case 11: c += keyn[10]<<24;
		case 10: c += keyn[9] << 16;
		case 9:  c += keyn[8] << 8;
		case 8:  b += keyn[7] << 24;
		case 7:  b += keyn[6] << 16;
		case 6:  b += keyn[5] << 8;
		case 5:  b += keyn[4];
		case 4:  a += keyn[3] << 24;
		case 3:  a += keyn[2] << 16;
		case 2:  a += keyn[1] << 8;
		case 1:  a += keyn[0];
	}
	OLIO_HASH_MIX(a, b, c);

#ifdef OLIO_DEBUG_HASH
	printf(" => %08x\n", c);
#endif

	return c;
}
