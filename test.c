#include <stdlib.h>
#include <stdio.h>

//#define VERY_VERBOSE

#define MSG(x) { printf(x); fflush(stdout); }
#define MSGF(x) { printf x ; fflush(stdout); }

#include "buffer.h"

static void test_buffer()
{
}

#include "array.h"

static void test_array()
{
	int k;
	const int * p;
	const int * q;
	OLIO_ARRAY_STACK(st, int, 64);
	olio_array * hp;

	OLIO_ARRAY_ALLOC(hp, int, 0);

	printf("[testing array...]\n"); fflush(stdout);

	MSGF(("st %u\n", st->base.allocated));
	MSGF(("hp %u\n", hp->base.allocated));

#define ARRAY_COUNT 1000

	MSG("appending to arrays...\n");
	for (k = 0; k < ARRAY_COUNT; k++) {
		olio_array_append(st, &k, 1);
		olio_array_append(hp, &k, 1);
	}

	MSG("prepending to arrays...\n");
	for (k = 0; k < ARRAY_COUNT; k++) {
		olio_array_prepend(st, &k, 1);
		olio_array_prepend(hp, &k, 1);
	}

	MSG("inserting to arrays...\n");
	for (k = 0; k <= ARRAY_COUNT * 2; k++) {
		olio_array_insert(st, k * 2, &k, 1);
		olio_array_insert(hp, k * 2, &k, 1);
	}

	MSG("checking array contents...\n");
	p = olio_array_contents(st);
	q = olio_array_contents(hp);
	for (k = 0; k < ARRAY_COUNT; k++) {
		if (p[k * 2 + 1] != ARRAY_COUNT - 1 - k ||
			p[k * 2 + 1 + ARRAY_COUNT * 2] != k)
			MSG("ERROR: odd numbered stack item not as expected\n");

		if (p[k * 2] != k ||
			p[k * 2 + ARRAY_COUNT * 2] != k + ARRAY_COUNT)
			MSG("ERROR: even numbered stack item not as expected\n");

		if (q[k * 2 + 1] != ARRAY_COUNT - 1 - k ||
			q[k * 2 + 1 + ARRAY_COUNT * 2] != k)
			MSG("ERROR: odd numbered heap item not as expected\n");

		if (q[k * 2] != k ||
			q[k * 2 + ARRAY_COUNT * 2] != k + ARRAY_COUNT)
			MSG("ERROR: even numbered heap item not as expected\n");
	}
	if (p[ARRAY_COUNT * 4] != ARRAY_COUNT * 2) 
		MSG("ERROR: last item in stack array not as expected\n");
	if (q[ARRAY_COUNT * 4] != ARRAY_COUNT * 2) 
		MSG("ERROR: last item in heap array not as expected\n");

#ifdef VERY_VERBOSE
	for (k = 0; k < olio_array_length(st) ||
		k < olio_array_length(hp); k++) {
		MSGF(("%i: st % 4i hp % 4i\n", k, 
			(k < olio_array_length(st)) ? p[k] : -1,
			(k < olio_array_length(hp)) ? q[k] : -1));
	}
#endif

	MSGF(("st %p = %p\n", st->base.data, st->stack));
	MSGF(("hp %p = %p\n", hp->base.data, hp->stack));

	MSG("freeing arrays...\n");
	olio_array_free(st);
	olio_array_free(hp);
}

#include "string.h"

static void test_string()
{
}

#include "graph.h"

static void test_graph()
{
	int rc;
	olio_graph g;

	printf("[testing graph...]\n"); fflush(stdout);

	printf("graph initialization...\n"); fflush(stdout);
	olio_graph_init(&g, 10);
	
	printf("graph setting edges...\n"); fflush(stdout);
	olio_graph_set_edge(&g, 0, 1, 10);
	olio_graph_set_edge(&g, 1, 2, 10);

	printf("graph testing cyclic...\n"); fflush(stdout);
	rc = olio_graph_acyclic(&g);
	if (rc == 0) {
		printf("ERROR: graph NOT acyclic\n"); fflush(stdout);
	}

	printf("graph setting edges...\n"); fflush(stdout);
	olio_graph_set_edge(&g, 2, 0, 10);

	MSG("graph testing cyclic...\n");
	rc = olio_graph_acyclic(&g);
	if (rc != 0) MSG("ERROR: graph NOT cyclic\n");

	MSG("freeing graph...\n");
	olio_graph_free(&g);
}

#include "skiplist.h"

static void test_skiplist()
{
	uint32_t test_data[] = {0xf, 0x436, 0x53547, 0x65, 0x54, 0x5654, 0x8756, 0x8767};

	olio_skiplist sk;
	int i;

	printf("sizeof(entry) = %li\n", sizeof(olio_skiplist_entry));
	printf("sizeof(block) = %li\n", sizeof(olio_skiplist_block));
	printf("sizeof(skiplist) = %li\n", sizeof(olio_skiplist));

	olio_skiplist_init(&sk);
	printf("adding...\n");
	for (i = 0; i < sizeof(test_data) / sizeof(uint32_t); i++) {
		int rv = olio_skiplist_add(&sk, test_data[i], (void*) test_data[i]);
		printf("k: 0x%x, rv: %i\n", test_data[i], rv);
	}

	printf("searching...\n");
	for (i = 0; i < sizeof(test_data) / sizeof(uint32_t); i++) {
		void * d = olio_skiplist_find(&sk, test_data[i]);
		printf("k: 0x%x, data: %p\n", test_data[i], d);
	}

	olio_skiplist_display(&sk);

	printf("freeing...\n");
	olio_skiplist_free(&sk);

	olio_skiplist_display(&sk);

	olio_skiplist_init(&sk);
	printf("adding...\n");
	for (i = 0; i < sizeof(test_data) / sizeof(uint32_t); i++) {
		int rv = olio_skiplist_add(&sk, test_data[i], (void*) test_data[i]);
		printf("k: 0x%x, rv: %i\n", test_data[i], rv);
	}
	
	printf("deleting...\n");
	for (i = 0; i < sizeof(test_data) / sizeof(uint32_t); i++) {
		void * d;
		int rv = olio_skiplist_remove(&sk, test_data[i], &d);
		printf("k: 0x%x, rv: %i, data: %p\n", test_data[i], rv, d);
		olio_skiplist_display(&sk);
	}

	printf("freeing...\n");
	olio_skiplist_free(&sk);

	olio_skiplist_display(&sk);
}

#include "error.h"

static void test_error()
{
}

#ifdef HAVE_GSL
#include <gsl/gsl_rng.h>
#include "random.h"

gsl_rng * gsl;
olio_random olio;

static void test_random()
{
    uint32_t seed;
    uint32_t a, b;
    long i;
    
    printf("[testing random...]\n"); fflush(stdout);
    
    gsl = gsl_rng_alloc(gsl_rng_mt19937);
    
    //for (seed = 0; seed < 1000; seed++) 
    {
	seed = 0;
	gsl_rng_set(gsl, 0);
	olio_random_set_seed(&olio, 4357);
	for (i = 0; i < 10000; i++) {
	    a = gsl_rng_uniform_int(gsl, 0xffffffff);
	    b = olio_random_integer(&olio);
	    if (a != b) {
		printf("mismatch\n");
	    }
	}
    }
    gsl_rng_free(gsl);
}
#else
static void test_random() {}
#endif

#include "phash.h"
#include "hash.h"

static void test_hashes()
{
    unsigned long i;
    const char * s[] = {
	"alpha", "beta", "gamma", "delta", "epsilon",
	"lamda", "mu", "nu", "omicron", "pi", "phi", "psi",
	"tau", "theta", "zeta", NULL
    };
    
    olio_phash_build m;
    int rc;
  
    printf("[testing hashes...]\n"); fflush(stdout);

    rc = olio_phash_init(&m, NULL);
    if (rc == -1) {
	printf("error, aborting\n");
	exit(1);
    }
    
    for (i = 0; s[i] != NULL; i++) {
	rc = olio_phash_add_entry(&m, s[i], strlen(s[i]), i * 1000);
	if (rc == -1) {
	    printf("error, aborting\n");
	    exit(1);
	}
    }
    
    rc = olio_phash_generate(&m, 1000, 1);
    if (rc < 0) {
	printf("error, aborting\n");
	exit(1);
    }
    if (rc > 0) {
	printf("failed to generate with attempt/extra limits\n");
	exit(1);
    }	
    
    printf("phash generated:\n");
    printf("A hash: %08x\nB hash: %08x\nG size: %u\n",
	   m.phash->a_hash_seed, m.phash->b_hash_seed, 
	   m.phash->g_table_size);
    
    printf("{ ");
    for (i = 0; i < m.phash->g_table_size; i++) {
	int16_t * gv = (int16_t *) m.phash->data; 
	printf("%i, ", gv[i]);
    }
    printf("}\n");
    
    for (i = 0; s[i] != NULL; i++) {
	int32_t v = olio_phash_value(m.phash,
				     s[i], strlen(s[i]));
	printf(" %s -> %li\n",
	       s[i], v);
    }
    
    olio_phash_free(&m);
}

#include "varint.h"

static void test_varint_sub(uint32_t x, uint8_t bytes_expected)
{
    uint32_t a;
    uint8_t storage[5];
    uint8_t bytes_set, bytes_get;
    uint8_t i;

    bytes_set = olio_varint_set(x, storage);
    a = olio_varint_get(storage, &bytes_get);

    if (a != x || bytes_set != bytes_get || bytes_set != bytes_expected) 
    {
	printf("x = %lu\n(%u) 0x", x, bytes_set);
	for (i = 0; i < bytes_set; i++)
	    printf("%02x", storage[i]);
	printf("\na = %lu\n(%u)\n\n", a, bytes_get);
    }
}


static void test_varint()
{
    printf("[testing varint...]\n"); fflush(stdout);

    test_varint_sub(0, 1);
    test_varint_sub(1,1);
    test_varint_sub(127, 1);
    test_varint_sub(200, 2);
    test_varint_sub(32000, 3);
    test_varint_sub(32000000, 3);
    test_varint_sub(100000000, 4);
    test_varint_sub(400000000, 5);
    test_varint_sub(0, 1);
    test_varint_sub(1, 1);
}

int main(int argv, char ** argc)
{
	test_buffer();
	test_array();
	test_string();
	test_graph();
	test_skiplist();
	test_error();
	test_random();
	test_hashes();
	test_varint();
}
