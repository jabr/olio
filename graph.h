#ifndef _OLIO_GRAPH_H_
#define _OLIO_GRAPH_H_

/* simple graph data structure and algorithms for quick update, 
 * traversal, and acyclic test 
 */

#include <inttypes.h>

typedef struct _olio_graph {
    /* directed edge connectivity is stored as a bitmap */
    uint8_t * connectivity;
    /* directed edge values between nodes as a 2D matrix */
    int32_t * matrix;
    /* tracking node color for certain graph alogrithms */
    uint8_t * node_color;
    uint16_t node_count;
} olio_graph;

/* type for graph traversal callback function */
typedef int(*olio_graph_traversal_cb)(olio_graph *, 
	uint16_t head, uint16_t tail,
	uint8_t color, void * user_data);

#ifdef __cplusplus
extern "C" {
#endif

int olio_graph_init(olio_graph *, uint16_t count);
void olio_graph_free(olio_graph *);
void olio_graph_reset(olio_graph *);
/* a depth first traversal of the graph, calling back for each node */
int olio_graph_dfs_traversal(olio_graph *, olio_graph_traversal_cb,
	void * user_data);
/* check if graph is acyclic */
int olio_graph_acyclic(olio_graph *);

#ifdef OLIO_DEBUG
void olio_graph_display(olio_graph *);
#endif

#ifdef __cplusplus
} /* extern C */
#endif

static inline void olio_graph_set_edge(olio_graph * g,
	uint16_t head, uint16_t tail, int32_t value)
{
    /* set bit for head->tail connectivity */
    g->connectivity[head * ((g->node_count + 0x7) >> 3) + (tail >> 3)]
	|= 1 << (tail & 0x7);
    /* store edge value */
    g->matrix[head * g->node_count + tail] = value;
}

static inline void olio_graph_set_edge_nondirected(olio_graph * g,
	uint16_t node1, uint16_t node2, int32_t value)
{
    olio_graph_set_edge(g, node1, node2, value);
    olio_graph_set_edge(g, node2, node1, value);
}

static inline void olio_graph_unset_edge(olio_graph * g,
	uint16_t head, uint16_t tail)
{
    /* clear bit for head->tail connectivity */
    g->connectivity[head * ((g->node_count + 0x7) >> 3) + (tail >> 3)]
	&= ~(1 << (tail & 0x7));
    /* reset edge value */
    g->matrix[head * g->node_count + tail] = 0;
}

static inline void olio_graph_unset_edge_nondirected(olio_graph * g,
	uint16_t node1, uint16_t node2)
{
    olio_graph_unset_edge(g, node1, node2);
    olio_graph_unset_edge(g, node2, node1);
}

static inline int olio_graph_is_edge_set(olio_graph * g,
	uint16_t head, uint16_t tail)
{
    /* test for head->tail connectivity */
    return (g->connectivity[head * ((g->node_count + 0x7) >> 3) + 
			    (tail >> 3)] & (1 << (tail & 0x7)));
}

static inline int32_t olio_graph_get_edge(olio_graph * g,
	uint16_t head, uint16_t tail)
{
    return (g->matrix[head * g->node_count + tail]);
}

#endif /* _OLIO_GRAPH_H_ */
