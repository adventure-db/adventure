#ifndef ADV_GRAPH_H
#define ADV_GRAPH_H

typedef struct edge_t
{
	long edgeId;
	void *kv;
	struct node_t *start;
	struct node_t *end;
} edge_t;

typedef struct node_t
{
	long nodeId;
	void *kv;
	struct edge_t *edge;
	long d;
} node_t;

/* 	Graph type
	Contains a 'birds-eye' view of the database
*/
typedef struct graph_t
{
	/*	Array of nodes
		Indexed by nodeId (which should be dense)
	*/
	struct node_t *node;

	/* Number of nodes */
	long n;

} graph_t;

#endif	/* ADV_GRAPH_H */