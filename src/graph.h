#ifndef ADV_GRAPH_H
#define ADV_GRAPH_H

/* Data structures */
struct edge
{
	unsigned long type;
	void *kv;

	struct node *node;
	struct edge *next;
};

struct node
{
	// Key/value pairs (properties)
	void *kv;

	// Edges (these are offsets, not pointers)
	unsigned long to;
	unsigned long from;
};

struct graph
{
	// Array of nodes indexed by id
	struct node *nodes;

	// Size of the table
	unsigned long size;

	// Number of nodes
	unsigned long n;

	// TODO: add pool of free entries etc...

};

/* Helper functions */
void genGraph();
void debugPrint(struct graph g);

#endif
