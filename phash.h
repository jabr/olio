#ifndef _OLIO_PHASH_H_
#define _OLIO_PHASH_H_

/* generate a near-perfect hash (ie. generate a hash table and hashing 
   seeds for a set of given keys resulting in a table with no conflicts). 
   the maximum empty space (in multiples of key number) is given as a 
   parameter (0 = perfect hash). currently uses a non-deterministic 
   approach, with a max attempts option. 
*/

#include "array.h"
#include "random.h"
#include "string.h"

/* resulting hash table */
typedef struct _olio_phash {
  uint32_t a_hash_seed;
  uint32_t b_hash_seed;
  uint16_t entry_count;
  uint16_t g_table_size;
  uint32_t data[1];
} olio_phash;

/* data structure for constructing the phash */
typedef struct _olio_phash_build {
  olio_array entries;
  olio_array entry_data;
  olio_random * rng;
	
  uint8_t flags;

  olio_phash * phash;
} olio_phash_build;

#ifdef __cplusplus
extern "C" {
#endif

    int olio_phash_init(olio_phash_build *, olio_random *);
    void olio_phash_free(olio_phash_build *);
    
    int olio_phash_add_entry(olio_phash_build *, const void * key, 
			     uint16_t len, uint32_t value);
    
    /* generates the phash: attempts is maximums number of tries, and
       extra is the maximum empty entries allowed in resulting hash */
    int olio_phash_generate(olio_phash_build *, int attempts, 
			    uint8_t extra);

    uint32_t olio_phash_value(const olio_phash *, const void * key,
			      uint16_t len);

#ifdef __cplusplus
} /* extern C */
#endif

static inline int olio_phash_add_string(olio_phash_build * m, 
	const olio_string * s, int32_t value)
{
	return olio_phash_add_entry(m, olio_string_contents(s),
		olio_string_length(s), value);
}

static inline int olio_phash_add_array(olio_phash_build * m, 
	const olio_array * a, int32_t value)
{
	return olio_phash_add_entry(m, olio_array_contents(a),
		olio_array_length(a), value);
}

#endif /* _OLIO_PHASH_H_ */
