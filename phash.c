#include <stdlib.h>

#include "phash.h"
#include "graph.h"
#include "hash.h"

struct _olio_phash_entry {
    uint32_t position;
    uint32_t value;
    uint16_t length;
};

int olio_phash_init(olio_phash_build * m, olio_random * rng)
{
    m->flags = 0x0;
    m->rng = NULL;
    
    /* the phash we try to generate */
    m->phash = malloc(sizeof(olio_phash));
    if (m->phash == NULL) return -1;
    
    m->phash->a_hash_seed = 0x0;
    m->phash->b_hash_seed = 0x0;
    m->phash->g_table_size = 0;
    
    OLIO_ARRAY_INIT(m->entries, struct _olio_phash_entry);
    OLIO_ARRAY_INIT(m->entry_data, uint8_t);
    
    /* initialize the random number generator */
    if (rng == NULL) {
	m->rng = (olio_random *) malloc(sizeof(olio_random));
	if (m->rng == NULL) {
	    olio_phash_free(m);
	    return -1;
	}
	olio_random_generate_seed(m->rng);
	m->flags |= 0x1;
    } else m->rng = rng;
    
    return 0;
}

void olio_phash_free(olio_phash_build * m)
{
    if (m->phash != NULL) free(m->phash);
    olio_array_free(&m->entry_data);
    olio_array_free(&m->entries);
    if (m->rng != NULL && m->flags & 0x1) free(m->rng);
}

/* add a key/value before constructing the phash */
int olio_phash_add_entry(olio_phash_build * m, 
			 const void * key, uint16_t len,
			 uint32_t value)
{
    int rc;
    struct _olio_phash_entry ne;
    uint16_t i;
    uint16_t entries = olio_array_length(&m->entries);
    const struct _olio_phash_entry * e = 
	(const struct _olio_phash_entry *)
	olio_array_contents(&m->entries);
    const uint8_t * x = (const uint8_t *) 
	olio_array_contents(&m->entry_data);
    
    /* exceeds max size? */
    if (entries >= 32767) return 2;
    
    /* make sure this key is unique */
    for (i = 0; i < entries; i++) {
	if (len == e[i].length &&
	    memcmp(x + e[i].position, key, len) == 0)
	    return 1;
    }
    
    ne.position = olio_array_length(&m->entry_data);
    ne.length = len;
    ne.value = value;
    
    /* append the key data */
    rc = olio_array_append(&m->entry_data, key, len);
    if (rc != 0) return rc;
    
    /* append the key/value pair */
    rc = olio_array_append(&m->entries, &ne, 1);
    if (rc != 0) return rc;
    
    return 0;
}

/* this function is called during a traversal of the hash graph to assign
   key mapping values. we end with an array of signed integers the size 
   of the phash table. hashing a key with the two seeds gives you two 
   entries in this array, and adding those numbers together with give the
   position in the value array. this only works because the graph of hash 
   connecitivies is a tree (ie. not cyclical) */
static int _assign_node_values(olio_graph * g, 
			       uint16_t head, uint16_t tail,
			       uint8_t color, void * user_data)
{
    olio_phash_build * m = (olio_phash_build *) user_data;
    int16_t * gv = (int16_t *) (m->phash->data + m->phash->entry_count);
    /* the value of the head will already be assigned (by previous
       connectivity, or a default of zero), so the value for the tail
       is simplly the edge value minus the head -- add the two (head and 
       tail) together and you get the original edge value (ie. the value 
       entry position corresponding to a given key) */
    int32_t v = olio_graph_get_edge(g, head, tail) - (int32_t) gv[head];
    /* if the value is out of bounds, abort */
    if (v < -32768 || v > 32767) return 1;
    gv[tail] = (int16_t) v;
    return 0;				
}

int olio_phash_generate(olio_phash_build * m, 
			int attempts, uint8_t extra)
{
    int rc;
    int i;
    uint16_t j;
    uint16_t entries = olio_array_length(&m->entries);
    olio_graph g;
    
    const struct _olio_phash_entry * e = 
	(const struct _olio_phash_entry *)
	olio_array_contents(&m->entries);
    const uint8_t * x = (const uint8_t *) 
	olio_array_contents(&m->entry_data);
    
    /* table size is number of entries times (extra + 1),
       rounded up to a multiple of 64 */
    m->phash->g_table_size = 
	(entries * ((uint16_t) extra + 1) + 0x3f) & 0xffc0;
    m->phash->entry_count = entries;
    
    /* prepare a graph to hashing conflicts */
    rc = olio_graph_init(&g, m->phash->g_table_size);
    if (rc != 0) return rc;
    
    /* attempt to generate a phash */
    i = 0;
    for (;;) {
	uint32_t ah, bh;
	/* choose random hashing seeds */
	m->phash->a_hash_seed = olio_random_integer(m->rng);
	m->phash->b_hash_seed = olio_random_integer(m->rng);
	
	for (j = 0; j < entries; j++) {
	    /* calculate hashes on 2 seeds */
	    ah = olio_hash_value(x + e[j].position, 
				 e[j].length, m->phash->a_hash_seed) 
		% m->phash->g_table_size;
	    bh = olio_hash_value(x + e[j].position, 
				 e[j].length, m->phash->b_hash_seed) 
		% m->phash->g_table_size;
	    /* check if hash combination is in conflict... */
	    if (ah == bh || olio_graph_is_edge_set(&g, ah, bh)) break;
	    /* if not, mark it as used, 
	       with edge value as key entry number */
	    olio_graph_set_edge_nondirected(&g, ah, bh, j);
	}

	/* did we manage to assign all the entries,
	   without making a cyclic graph? */
	if (j == entries && olio_graph_acyclic(&g)) break;

	/* nope, let's try again with new hashing seeds */
	olio_graph_reset(&g);
	
	i++;
	if (attempts > 0 && i == attempts) {
	    /* too many attempts, we give up */
	    olio_graph_free(&g);
	    return 1;
	}
    }

    /* allocate space for our phash */
    m->phash = (olio_phash *)
	realloc(m->phash, sizeof(olio_phash) - sizeof(uint32_t) +
		(sizeof(uint32_t) * m->phash->entry_count) +
		(sizeof(int16_t) * m->phash->g_table_size));
    if (m->phash == NULL) return -1;

    /* initialize the phash data to zero */
    memset(m->phash->data + m->phash->entry_count, 0, 
	   sizeof(int16_t) * m->phash->g_table_size);
    
    /* assign values to the hash entries */
    for (j = 0; j < m->phash->entry_count; j++)
	m->phash->data[j] = e[j].value;

    /* create the hash mapping array for this graph */
    rc = olio_graph_dfs_traversal(&g, _assign_node_values, m);

    olio_graph_free(&g);
    return rc;
}

/* retrieve the value for a key from a phash */
uint32_t olio_phash_value(const olio_phash * p, const void * key,
			  uint16_t len)
{
    /* hash the key with the two seeds */
    uint32_t a = olio_hash_value(key, len, p->a_hash_seed);
    uint32_t b = olio_hash_value(key, len, p->b_hash_seed);
    int16_t * gv = (int16_t *) (p->data + p->entry_count);
    /* retrieve the entry number corresponding to those two hashes */
    int16_t i = gv[a % p->g_table_size] + gv[b % p->g_table_size];
    /* is it out of bounds? */
    if (i < 0 || i >= p->entry_count) return 0xffffffff;
    /* return the value */
    return p->data[i];
}
