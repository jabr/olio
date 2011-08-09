#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "array.h"

int olio_graph_init(olio_graph * g, uint16_t count)
{
    g->node_count = count;
    g->node_color = (uint8_t *) malloc(sizeof(uint8_t) * count);
    /* connectivity bitmap: effectively a count-sized array, each entry
       having count number of bits. with 8 bits per byte, we only need to
       allocated count * min(count/8, 1) bytes. */
    g->connectivity = (uint8_t *) malloc(sizeof(uint8_t) * count *
					 ((count + 0x7) >> 3));
    /* a 2D array holding edge values */
    g->matrix = (int32_t *) malloc(sizeof(int32_t) * count * count);

    if (g->node_color == NULL || g->connectivity == NULL ||
	g->matrix == NULL) {
	olio_graph_free(g);
	return -1;
    }
    olio_graph_reset(g);
    return 0;
}

void olio_graph_free(olio_graph * g)
{
    if (g->node_color != NULL) free(g->node_color);
    if (g->connectivity != NULL) free(g->connectivity);
    if (g->matrix != NULL) free(g->matrix);
}

void olio_graph_reset(olio_graph * g)
{
    /* reset connectivity bitmap... */
    memset(g->connectivity, 0, sizeof(uint8_t) * g->node_count *
	   ((g->node_count + 0x7) >> 3));
    /* and the edge values */
    memset(g->matrix, 0, sizeof(int32_t) * g->node_count * g->node_count);
}

struct edge_pair {
    uint16_t start;
    uint16_t end;
};

/* traverse graph with depth-first search, calling "cb" with each edge we find.
   this is a stack-based, non-recursive implementation. further, the connectivity
   bitmap provides excellent cache coherency on the inner loop edge test. */
int olio_graph_dfs_traversal(olio_graph * g, olio_graph_traversal_cb cb,
			     void * user_data)
{
    OLIO_ARRAY_STACK(visit, struct edge_pair, 64);
    uint16_t i, j;
    struct edge_pair a, b;
    int rc = 0;

    /* reset node colors (used for tracking visit state) */
    memset(g->node_color, 0, sizeof(uint8_t) * g->node_count);
    
    for (i = 0; i < g->node_count; i++) {
        /* has node been visited? */
	if (g->node_color[i] == 0) { 
	    /* start a new edge stack */
	    a.start = 0xffff;
	    a.end = i;
	    olio_array_prepend(visit, &a, 1);

	    /* loop while we have edges on the stack to check... */
	    while (olio_array_length(visit) > 0) {
		struct edge_pair * t = 
		    (struct edge_pair *) 
		    olio_array_contents(visit);
		a.start = t->start;
		a.end = t->end;
		olio_array_remove(visit, 0, 1);
		/* mark this node as "visited from" */
		g->node_color[a.end] = 1;
		
		/* find nodes this node is connected to */
		for (j = 0; j < g->node_count; j++) {
		    /* don't go backwards */
		    if (j == a.start) continue;

		    if (olio_graph_is_edge_set(g, a.end, j)) {
			/* a connected node. have we seen it before? */
	    		if (g->node_color[j] == 0) {
			    /* no. add this edge to the stack */
			    b.start = a.end;
			    b.end = j;
			    olio_array_prepend(visit, &b, 1);
	    		}
			/* call callback with the edge */
		    	rc = cb(g, a.end, j, g->node_color[j], user_data);
		    	if (rc != 0) {
			    /* the callback aborted the traversal. */
			    goto end_dfs_traversal;
		    	}
			/* mark this node as "visited to" */
		    	g->node_color[j] = 2;
		    }
	   	}
	    }
	}
    }
    
end_dfs_traversal:
    olio_array_free(visit);
    
    return rc;
}

static int _acyclic_test(olio_graph * g, uint16_t head, uint16_t tail, 
			 uint8_t color, void * user_data)
{
    /* if we find an edge that points to a node we've already seen before,
       then the graph is cyclic, so we abort the traversal with the status. */
    if (color != 0) return 1;
    return 0;
}

/* check if the graph is acyclic by doing a dfs traversal with the callback above. */
int olio_graph_acyclic(olio_graph * g)
{
    int rc = olio_graph_dfs_traversal(g, _acyclic_test, NULL);
    return (rc == 0);
}

#ifdef OLIO_DEBUG

#include <stdio.h>

void olio_graph_display(olio_graph * g)
{
    uint16_t i, j;

    /* print a formatted matrix of the graph's connectivity/edge values */
    for (i = 0; i < g->node_count; i++) {
	for (j = 0; j < g->node_count; j++) {
	    if (olio_graph_is_edge_set(g, i, j)) {
		fprintf(stderr, "% 3li ", olio_graph_get_edge(g, i, j));
	    } else {
		fprintf(stderr, "  . ");
	    }
	}
	fprintf(stderr, "\n");
    }
}

#endif /* OLIO_DEBUG */
