#include "edges.h"

dbf_p edges_add(struct dbf *edges, dbf_p e_old, nodeid_t start, nodeid_t end, uint64_t type)
{
	dbf_p e_new, size;

	if(e_old == 0) {
		size = 8;
		e_new = dbf_alloc(edges, size + 8);
	} else {
		size = dbf_read_ui64(edges, e_old);
		e_new = dbf_alloc(edges, size + 8);
		dbf_copy(edges, e_new, e_old, size);
	}

	dbf_write_ui64(edges, e_new + size, end);
	dbf_write_ui64(edges, e_new, size + 8);
	dbf_free(edges, e_old);
	return e_new;
}

dbf_p edges_remove(struct dbf *edges, dbf_p e_old, nodeid_t start, nodeid_t end)
{
	if(e_old == 0) return 0;

	dbf_p hdr_sz = 8;
	dbf_p item_sz = 8;

	dbf_p size = dbf_read_ui64(edges, e_old);
	dbf_p i;
	for(i = hdr_sz; i < size; i += item_sz) {
		if( dbf_read_ui64(edges, e_old + i) == end ) break;
	}

	if(i >= size) return e_old;

	int64_t size_new = size - item_sz;
	if(size_new < 0) return e_old;

	dbf_p e_new = dbf_alloc(edges, size_new);
	dbf_copy(edges, e_new, e_old, i);
	dbf_copy(edges, e_new + i, e_old + i + item_sz, size - i - item_sz);

	dbf_write_ui64(edges, e_new, size - item_sz);
	dbf_free(edges, e_old);
	return e_new;
}
