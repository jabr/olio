#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "random.h"

/* This is an implementation of the Mersenne Twister random number
 * generator with the 19937 revision of the seeding procedure.
 * It was ported to C from the public domain lisp implemenation 
 * contained in CMUCL. */
 
#define MT19937_N 624
#define MT19937_M 397
#define MT19937_UPPER_MASK 0x80000000UL
#define MT19937_LOWER_MASK 0x7fffffffUL
#define MT19937_B 0x9d2c5680UL
#define MT19937_C 0xefc60000UL

void olio_random_set_seed(olio_random * r, uint32_t seed)
{
	int i;
	r->seed = seed;
	r->state[0] = 0;
	r->state[1] = 0x9908b0df;
	r->state[2] = MT19937_N;
	r->state[3] = seed & 0xffffffffUL;
	for (i = 4; i < MT19937_N + 3; i++)
		r->state[i] = (1812433253 * 
			(r->state[i-1] ^ 
			(r->state[i-1] >> 30)) + i - 3) 
			& 0xffffffffUL;
}

uint32_t olio_random_integer(olio_random * r)
{
	uint32_t a;
	int i;
	if (r->state[2] == MT19937_N) {
		for (i = 3; i < MT19937_N - MT19937_M + 3; i++) {
			a = (r->state[i] & MT19937_UPPER_MASK) |
				(r->state[i+1] & MT19937_LOWER_MASK);
			r->state[i] = r->state[i + MT19937_M] ^
				(a >> 1) ^ r->state[a & 0x00000001UL];
		}
		for (; i < MT19937_N + 3 - 1; i++) {
			a = (r->state[i] & MT19937_UPPER_MASK) |
				(r->state[i+1] & MT19937_LOWER_MASK);
			r->state[i] = r->state[i + MT19937_M - MT19937_N] ^
				(a >> 1) ^ r->state[a & 0x00000001UL];
		}
		a = (r->state[MT19937_N + 3 - 1] & MT19937_UPPER_MASK) |
			(r->state[3] & MT19937_LOWER_MASK);
		r->state[MT19937_N + 3 - 1] =
			r->state[MT19937_M + 3 - 1] ^ (a >> 1) ^
			r->state[a & 0x00000001UL];
		r->state[2] = 0;
	}
	i = r->state[2]++;
	a = r->state[i+3];
	a ^= a >> 11;
	a ^= (a & (MT19937_B >> 7)) << 7;
	a ^= (a & (MT19937_C >> 15)) << 15;
	a ^= a >> 18;
	return a;
}

uint32_t olio_random_generate_seed(olio_random * r)
{
	uint32_t seed;
	FILE * noise;
	struct timeval tv;

	/* some basic, changing variables to start with... */
	gettimeofday(&tv, NULL);
	seed = tv.tv_sec ^ tv.tv_usec ^ getpid() ^ getppid();

	/* get some noise from the current process states */
	noise = popen("ps axww | gzip", "r");
	if (noise != NULL) {
		unsigned int count = 0;
		uint32_t dw = 0;
		while (!feof(noise)) {
			dw = fgetc(noise) & 0x000000ffUL;
			dw <<= (count & 0x00000003UL) << 3;
			seed ^= dw;
			count++;
		}
		pclose(noise);
	}

	/* get some noise from the contents of the /tmp directory */
	noise = popen("ls -al /tmp | gzip", "r");
	if (noise != NULL) {
		unsigned int count = 0;
		uint32_t dw = 0;
		while (!feof(noise)) {
			dw = fgetc(noise) & 0x000000ffUL;
			dw <<= (count & 0x00000003UL) << 3;
			seed ^= dw;
			count++;
		}
		pclose(noise);
	}

	/* get some noise from /proc/stat */
	noise = popen("cat /proc/stat | gzip", "r");
	if (noise != NULL) {
		unsigned int count = 0;
		uint32_t dw = 0;
		while (!feof(noise)) {
			dw = fgetc(noise) & 0x000000ffUL;
			dw <<= (count & 0x00000003UL) << 3;
			seed ^= dw;
			count++;
		}
		pclose(noise);
	}

	/* get some noise from /proc/meminfo */
	noise = popen("cat /proc/meminfo | gzip", "r");
	if (noise != NULL) {
		unsigned int count = 0;
		uint32_t dw = 0;
		while (!feof(noise)) {
			dw = fgetc(noise) & 0x000000ffUL;
			dw <<= (count & 0x00000003UL) << 3;
			seed ^= dw;
			count++;
		}
		pclose(noise);
	}

	/* take 32 bytes from /dev/urandom */
	noise = fopen("/dev/urandom", "r");
	if (noise != NULL) {
		unsigned int count = 0;
		uint32_t dw = 0;
		while (!feof(noise) && count < 32) {
			dw = fgetc(noise) & 0x000000ffUL;
			dw <<= (count & 0x00000003UL) << 3;
			seed ^= dw;
			count++;
		}
		pclose(noise);
	}

	olio_random_set_seed(r, seed);
	return seed;
}
