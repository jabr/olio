#ifndef _OLIO_SKIPLIST_H_
#define _OLIO_SKIPLIST_H_

/* paged, deterministic skip list.
 * references:
 *  MacDonald JP & Zhao BY. "T-treap and Cache Performance of Indexing Data Structures."
 *  Munro J et al. "Deterministic Skip Lists." Proc. ACM-SIAM Symposium on Discrete Algo. 1992
 */

#include <inttypes.h>

#define OLIO_SKIPLIST_MAX_KEY 0xffffffff

typedef struct _olio_skiplist_entry {
	uint32_t key;
	union {
		struct _olio_skiplist_block * block;
		void * data;
	};
} olio_skiplist_entry;

#define OLIO_SKIPLIST_BLOCK_M 8
typedef struct _olio_skiplist_block {
	uint8_t leaf : 1;
	uint8_t unused : 7;
	uint8_t count;
	uint16_t unused2;
	struct _olio_skiplist_block * next;
	struct _olio_skiplist_entry entry[OLIO_SKIPLIST_BLOCK_M-1];
} olio_skiplist_block;

typedef struct _olio_skiplist {
	struct _olio_skiplist_block * head;
	uint32_t count;
	uint16_t height;
} olio_skiplist;

typedef struct _olio_skiplist_cursor {
	struct _olio_skiplist_block * block;
	uint8_t index;
} olio_skiplist_cursor;

#ifdef __cplusplus
extern "C" {
#endif

/* basic init/free functions */
int olio_skiplist_init(olio_skiplist *);
void olio_skiplist_free(olio_skiplist *);

/* find the entry for a particular key */
void * olio_skiplist_find(const olio_skiplist *, uint32_t key);

/* prepare for a sequential search, with keys >= key */
void olio_skiplist_range_begin(const olio_skiplist *, uint32_t key, 
	olio_skiplist_cursor * cursor);
/* iterate on the sequential search, with keys <= mkey */
void * olio_skiplist_range_next(const olio_skiplist *, uint32_t mkey,	
	uint32_t * ckey, olio_skiplist_cursor * cursor);

/* add an entry */
int olio_skiplist_add(olio_skiplist *, uint32_t key, void * data);
/* remove an entry */
int olio_skiplist_remove(olio_skiplist *, uint32_t key, void ** data);

#ifdef OLIO_DEBUG
/* show the entries (and keys) at each height level */
void olio_skiplist_display(const olio_skiplist * sk);
#endif

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _OLIO_SKIPLIST_H_ */
