#ifndef ADV_DB_STORE_EDGES
#define ADV_DB_STORE_EDGES

#include "dbf.h"
#include "../db.h"

dbf_p edges_add(struct dbf *edges, dbf_p e_old, nodeid_t start, nodeid_t end, uint64_t type);
dbf_p edges_remove(struct dbf *edges, dbf_p e_old, nodeid_t start, nodeid_t end);

#endif
