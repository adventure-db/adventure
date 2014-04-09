#include "edges.h"

edge_p edges_add(struct store *edges, edge_p e_old, node_p start, node_p end)
{
	edge_p e_new, size;

	if(e_old == 0) {
		size = 8;
		e_new = store_alloc(edges, size + 8);
	} else {
		size = store_read_ui64(edges, e_old);
		e_new = store_alloc(edges, size + 8);
		store_copy(edges, e_new, e_old, size);
	}

	store_write_ui64(edges, e_new + size, end);
	store_write_ui64(edges, e_new, size + 8);
	store_free(edges, e_old);
	return e_new;
}

edge_p edges_remove(struct store *edges, edge_p e_old, node_p start, node_p end)
{
	if(e_old == 0) return 0;

	edge_p hdr_sz = 8;
	edge_p item_sz = 8;

	edge_p size = store_read_ui64(edges, e_old);
	edge_p i;
	for(i = hdr_sz; i < size; i += item_sz) {
		if( store_read_ui64(edges, e_old + i) == end ) break;
	}

	if(i >= size) return e_old;

	int64_t size_new = size - item_sz;
	if(size_new < 0) return e_old;

	edge_p e_new = store_alloc(edges, size_new);
	store_copy(edges, e_new, e_old, i);
	store_copy(edges, e_new + i, e_old + i + item_sz, size - i - item_sz);

	store_write_ui64(edges, e_new, size - item_sz);
	store_free(edges, e_old);
	return e_new;
}
