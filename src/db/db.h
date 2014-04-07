#ifndef ADV_DB_DB_H
#define ADV_DB_DB_H

#include <stdint.h>

#include <lib/sds/sds.h>
#include "store/dbf.h"
#include "store/sa.h"

#define ADV_CREATE 0x1

typedef uint64_t nodeid_t;

struct adv_node
{
	uint64_t p_edge;
	uint64_t p_kv;
};

struct adv_edge
{
	uint64_t type;
	uint64_t edge_id;
	uint64_t p_enode;
};

struct adv_db
{
	sds path;

	struct dbf *nodes;
	struct dbf *edges;
};

// Open/close an existing database from disk
struct adv_db *adv_open(const char *dbname, int flags);
void adv_close(struct adv_db *db);

nodeid_t adv_node_add(struct adv_db *db);
int adv_edge_add(struct adv_db *db, nodeid_t start, nodeid_t end, uint64_t type);
int adv_edge_remove(struct adv_db *db, nodeid_t start, nodeid_t end);

// Debug functions
void adv_debug_print(struct adv_db *db);

#endif
