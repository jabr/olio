#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>

/* a wavelet-based, k-means 2D clustering algorithm */

#include "evaluate.h"

#ifdef DEBUG_CLUSTER
// for debugging and performance analysis
static void dump_clusters(evaluate * e)
{
    int i;
    
    printf("X clusters\n");
    for (i = 0; i < e->current_num_clusters; i++) {
	printf("%i: %lf +/- %lf (%lu)\n", i, e->clusters[0][i].mean,
	       sqrt(e->clusters[0][i].variance),
	       e->clusters[0][i].count);
    }
    
    printf("F clusters\n");
    for (i = 0; i < e->current_num_clusters; i++) {
	printf("%i: %lf +/- %lf (%lu)\n", i, e->clusters[1][i].mean,
	       sqrt(e->clusters[0][i].variance),
	       e->clusters[1][i].count);
    }
}
#else
#define dump_clusters(e)
#endif

// max number of refinement passes to take
#define MAX_REFINE_PASSES 10

void evaluate_cluster_kmean_wavelet(evaluate * e)
{
    uint32_t i;
    uint32_t j;
    uint32_t k;
    uint32_t l;
    uint32_t m;
    uint32_t n;
    
    int a = 0;
    int ai;
    int b;
    
    // decompose dataset
    // array is iteratively replaced by tuples of the average and deviance 
    // from avg for two given, likely disjoint values in the array.
    while (e->current_num_datapoints >> (a + 1) >= e->current_num_clusters) 
    {
	// proper array positioning
	l = 1 << a;
	j = l << 1;
	k = e->current_num_datapoints - l;
	a++;
	
	for (i = 0; i < k; i += j) {
	    // calculate midpoints and average deviance
	    float xa = 0.5 * (e->datapoints[i].x + e->datapoints[i+l].x);
	    float xd = 0.5 * (e->datapoints[i].x - e->datapoints[i+l].x);
	    float fa = 0.5 * (e->datapoints[i].f + e->datapoints[i+l].f);
	    float fd = 0.5 * (e->datapoints[i].f - e->datapoints[i+l].f);
	    // replace existing values with avg and dev
	    e->datapoints[i].x = xa;
	    e->datapoints[i+l].x = xd;
	    e->datapoints[i].f = fa;
	    e->datapoints[i+l].f = fd;
	}
    }
    
    ai = a;
    
    // reset cluster values
    for (m = 0; m < e->current_num_datapoints >> a; m++) 
    {
	e->datapoints[m << a].cluster_x =
	    e->datapoints[m << a].cluster_f = 0xff;
    }
    
    // compose and refine clusters
    for (;;) {
	int refine = 1;
	int passes;
	// refine current (or define initial) clusters
	for (passes = 0; refine && passes < MAX_REFINE_PASSES; passes++)
	{
	    for (b = 0; b < e->current_num_clusters; b++) {
		// x-axis
		// determine current mean and variance for this cluster
		if (e->clusters[0][b].count) {
		    e->clusters[0][b].mean = 
			e->clusters[0][b].sum / (float) e->clusters[0][b].count;
		    if (e->clusters[0][b].count > 1)
			e->clusters[0][b].variance =
			    (e->clusters[0][b].ssum -
			     e->clusters[0][b].sum * e->clusters[0][b].sum /
			     (double) e->clusters[0][b].count) /
			    ((double) e->clusters[0][b].count - 1.0);
		    else
			e->clusters[0][b].variance = 0.0;
		} else {
		    // no datapoints for this cluster, so assign a random mean.
		    n = random() % (e->current_num_datapoints >> ai);
		    e->clusters[0][b].mean = e->datapoints[n << ai].x;
		    e->clusters[0][b].variance = 0.0;
		}
		// f-axis
		// ditto: find mean and variance
		if (e->clusters[1][b].count) {
		    e->clusters[1][b].mean = 
			e->clusters[1][b].sum / (float) e->clusters[1][b].count;
		    if (e->clusters[1][b].count > 1)
			e->clusters[1][b].variance =
			    (e->clusters[1][b].ssum -
			     e->clusters[1][b].sum * e->clusters[1][b].sum /
			     (double) e->clusters[1][b].count) /
			    ((double) e->clusters[1][b].count - 1.0);
		    else
			e->clusters[1][b].variance = 0.0;
		} else {
		    // or just make something up for now
		    n = random() % (e->current_num_datapoints >> ai);
		    e->clusters[1][b].mean = e->datapoints[n << ai].f;
		    e->clusters[1][b].variance = 0.0;
		}
		e->clusters[0][b].sum = e->clusters[1][b].sum = 0.0;
		e->clusters[0][b].ssum = e->clusters[1][b].ssum = 0.0;
		e->clusters[0][b].count = e->clusters[1][b].count = 0;
	    }
	    
	    refine = 0;

	    // for each data point...
	    for (m = 0; m < e->current_num_datapoints >> a; m++) 
	    {
		float x = e->datapoints[m << a].x;
		float f = e->datapoints[m << a].f;
		float t;
		
		int c_x = 0;
		int c_f = 0;

		// find deviance from the initial cluster's mean
		float d_x = fabs(e->clusters[0][0].mean - x);
		float d_f = fabs(e->clusters[1][0].mean - f);

		// find which cluster it is closest to...
		for (b = 1; b < e->current_num_clusters; b++) {
		    t = fabs(e->clusters[0][b].mean - x);
		    if (t < d_x) { d_x = t; c_x = b; }
		    t = fabs(e->clusters[1][b].mean - f);
		    if (t < d_f) { d_f = t; c_f = b; }
		}
		
		// alter the closest cluster to include this data point
		
		// on the x-axis
		e->clusters[0][c_x].sum += x;
		e->clusters[0][c_x].ssum += x * x;
		e->clusters[0][c_x].count++;
		if (e->datapoints[m << a].cluster_x != c_x) refine = 1;
		e->datapoints[m << a].cluster_x = c_x;
		
		// and on the f-axis
		e->clusters[1][c_f].sum += f;
		e->clusters[1][c_f].ssum += f * f;
		e->clusters[1][c_f].count++;
		if (e->datapoints[m << a].cluster_f != c_f) refine = 1;
		e->datapoints[m << a].cluster_f = c_f;
	    }
	}
	
	// we can't recompose the dataset to a higher level than we
	// started with initially...
	if (a == 0 || a <= ai >> 2) break;
	
	// array positioning again
	a--;
	l = 1 << a;
	j = l << 1;
	k = e->current_num_datapoints - l;
	
	// and we recompose the datapoints
	for (i = 0; i < k; i += j) {
	    // the x
	    float x1 = e->datapoints[i].x + e->datapoints[i+l].x;
	    float x2 = e->datapoints[i].x - e->datapoints[i+l].x;
	    // the f
	    float f1 = e->datapoints[i].f + e->datapoints[i+l].f;
	    float f2 = e->datapoints[i].f - e->datapoints[i+l].f;
	    
	    // and make it so:
	    e->datapoints[i].x = x1;
	    e->datapoints[i+l].x = x2;
	    e->datapoints[i+l].cluster_x = e->datapoints[i].cluster_x;
	    e->datapoints[i].f = f1;
	    e->datapoints[i+l].f = f2;
	    e->datapoints[i+l].cluster_f = e->datapoints[i].cluster_f;
	}
    }

    // normalize
    for (i = 0; i < e->current_num_clusters; i++) {
	e->clusters[0][i].count <<= a;
	e->clusters[1][i].count <<= a;
    }
    
    // reset the return value
    memset(e->histogram, 0, sizeof(uint32_t) * 
	   e->current_num_clusters * e->current_num_clusters);

    // and assign weights to the resulting clusters
    for (i = 0; i < e->current_num_datapoints >> a; i++) {
	e->histogram[e->datapoints[i << a].cluster_x * e->current_num_clusters +
		     e->datapoints[i << a].cluster_f] += 1 << a;
    }
}
