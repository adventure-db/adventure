#ifndef ADV_GRAPH_H
#define ADV_GRAPH_H

typedef unsigned long ulong_t;
typedef long long_t;

/* Data structures */
typedef struct edge_t
{
	ulong_t id;
	void *kv;
	struct node_t *start;
	struct node_t *end;
} edge_t;

typedef struct node_t
{
	// Node id number (primary key)
	ulong_t id;

	// Flags (can be used by traversal algos)
	long_t flags;

	// Key/value pairs (properties)
	void *kv;

	// Edges
	struct edge_t *edges;

	// Number of edges
	ulong_t e;

} node_t;

typedef struct graph_t
{
	// Array of node pointers index by id
	struct node_t **nodes;

	// Size of the table
	ulong_t size;

	// Number of nodes
	ulong_t n;

	// TODO: add pool of free entries etc...

} graph_t;

/* Helper functions */
void debugPrint(graph_t g);

#endif	/* ADV_GRAPH_H */