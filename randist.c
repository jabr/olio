#include <math.h>

#include "randist.h"

double olio_randist_gaussian(olio_random * r, double sigma)
{
	double x, y, w;

	do {
		x = 2.0 * olio_random_real_ii(r) - 1.0;
		y = 2.0 * olio_random_real_ii(r) - 1.0;
		w = x * x + y * y;
	} while (w >= 1.0 || w <= 0);
	
	return sigma * y * sqrt((-2.0 * log(w)) / w);
}

uint32_t olio_randist_poisson(olio_random * r, double lamda)
{
	uint32_t i = 0;
	double z = exp(-lamda);
	double a = olio_random_real_ii(r);
	while (i != 0xffffffffUL && a > z) {
		i++;
		a *= olio_random_real_ii(r);
	}
	return i;
}

uint16_t olio_randist_discrete(olio_random * r, const double * p, uint16_t n)
{
	uint16_t i = 0;
	double s = 0.0, t;
	while (i < n) s += p[i++];
	t = olio_random_real_ie(r) * s;
	i = 0;
	while (t > p[i] && i < n - 1) t -= p[i++];
	return i;
}
