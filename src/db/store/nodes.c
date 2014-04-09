#include "nodes.h"

#include <src/util/dbg.h>

#define NODES_HEADER_SZ 32
#define NODES_ITEM_SZ 16

node_p nodes_count(struct store *nodes)
{
	return (nodes->end - NODES_HEADER_SZ) / NODES_ITEM_SZ;
}

edge_p nodes_edges_from(struct store *nodes, node_id node)
{
	return store_read_ui64(nodes, NODES_HEADER_SZ + NODES_ITEM_SZ * node);
}

int nodes_edges_set(struct store *nodes, node_id node, edge_p e)
{
	store_write_ui64(nodes, NODES_HEADER_SZ + NODES_ITEM_SZ * node, e);
	return 0;
}

node_id nodes_add(struct store *nodes)
{
	node_id node = (nodes->end - NODES_HEADER_SZ) / NODES_ITEM_SZ;
	debug("Adding node %llu", node);

	store_p p = store_alloc(nodes, NODES_ITEM_SZ);

	// Pointer to edge segment
	store_write_ui64(nodes, p, 0);

	// Pointer to key/value segment 
	store_write_ui64(nodes, p + 8, 0);

	return node;
}
