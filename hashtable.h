#ifndef _OLIO_HASHTABLE_H_
#define _OLIO_HASHTABLE_H_

/* bucket-based cuckoo hash table.
 * references:
 */

#include <inttypes.h>

#define OLIO_HASHTABLE_BUCKET_SIZE 8 /* with 8 byte entries, this fits in a common 64-byte cache line */

typedef struct _olio_hashtable_bucket {
  struct {
    uint32_t key;
    void * data;
  } entries[OLIO_HASHTABLE_BUCKET_SIZE];
} olio_hashtable_bucket;

typedef struct _olio_hashtable {
	uint32_t count;
	uint32_t bucket_count;
  struct _olio_hashtable_bucket buckets[1];
} olio_hashtable;

#ifdef __cplusplus
extern "C" {
#endif

/* basic init/free functions */
int olio_hashtable_init(olio_hashtable *);
void olio_hashtable_free(olio_hashtable *);

/* find the entry for a particular key */
void * olio_hashtable_find(const olio_hashtable *, uint32_t key);

/* add an entry */
int olio_hashtable_add(olio_hashtable *, uint32_t key, void * data);
/* remove an entry */
int olio_hashtable_remove(olio_hashtable *, uint32_t key, void ** data);

#ifdef OLIO_DEBUG
/* show the entries (and keys) */
void olio_hashtable_display(const olio_hashtable * ht);
#endif

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _OLIO_HASHTABLE_H_ */
