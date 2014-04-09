#ifndef ADV_DB_STORE_EDGES
#define ADV_DB_STORE_EDGES

#include "store.h"
#include "types.h"

/*	EDGE PAGE
	header
	edge item(s)

	EDGE ITEM
	TOTAL SIZE: 24 bytes
	node_id (8): end node id of this edge
	type (4): type of the edge
	wgt (4): weight of the edge
	kv (8): pointer to key/values of edge
*/

edge_p edges_add(struct store *edges, edge_p e_old, node_p start, node_p end);
edge_p edges_remove(struct store *edges, edge_p e_old, node_p start, node_p end);

#endif
