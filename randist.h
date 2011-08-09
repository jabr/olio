#ifndef _OLIO_RANDIST_H_
#define _OLIO_RANDIST_H_

#include "random.h"

#ifdef __cplusplus
extern "C" {
#endif

double olio_randist_gaussian(olio_random * r, double sigma);
uint32_t olio_randist_poisson(olio_random * r, double lamda);
uint16_t olio_randist_discrete(olio_random * r, 
	const double * p, uint16_t n);

#ifdef __cplusplus
} /* extern C */
#endif

static inline double olio_randist_gaussian_mean(olio_random * r, 
	double mean, double sigma)
{ return mean + olio_randist_gaussian(r, sigma); }

#endif /* _OLIO_RANDIST_H_ */
