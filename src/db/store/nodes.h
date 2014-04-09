#ifndef ADV_DB_STORE_NODES
#define ADV_DB_STORE_NODES

#include "store.h"
#include "types.h"

/*	NODE ITEM
	TOTAL SIZE: 16 bytes
	edge_p (8): pointer to edge segment
	store_p (8): pointer to k/v segment
*/

node_p nodes_count(struct store *nodes);

node_p nodes_add(struct store *nodes);
int nodes_remove(struct store *nodes, node_p node);

edge_p nodes_edges_from(struct store *nodes, node_p node);
int nodes_edges_set(struct store *nodes, node_p node, edge_p e);

#endif
