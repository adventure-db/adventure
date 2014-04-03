#ifndef ADV_STRUCT_H
#define ADV_STRUCT_H

// Data structures for database
struct edge
{
	unsigned long type;
	unsigned long p_kv;

	unsigned long p_node;
	unsigned long p_next;
};

struct node
{
	// Key/value pairs (properties)
	unsigned long p_kv;

	// Pointers to edges (these are offsets, aka relative pointers)
	unsigned long p_to;
	unsigned long p_from;
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

#endif
