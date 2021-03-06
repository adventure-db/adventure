#ifndef ADV_DB_ADV_H
#define ADV_DB_ADV_H

#include <stdint.h>

#include <lib/sds/sds.h>
#include "store/types.h"

#define ADV_CREATE STORE_O_CREAT

struct adv_meta
{
	// Useful heuristics
	node_p supernodes[10];
	
	// Statistics
	size_t node_count;
	size_t edge_count;
	size_t disk_sz;
	size_t node_file_sz;
	size_t edge_file_sz;
};

struct adv_txn
{
	uint64_t txn_id;
	// TODO: how should we represent dirty pages?
};

struct adv_db
{
	sds path;
	struct adv_meta meta;
	struct store *nodes;
	struct store *edges;
};

// Open/close an existing database from disk
struct adv_db *adv_open(const char *dbname, int flags);
void adv_close(struct adv_db *db);

// Maintenance
int adv_validate(struct adv_db *db);
int adv_repair(struct adv_db *db);
int adv_compact(struct adv_db *db);
int adv_backup(struct adv_db *dest, struct adv_db *src);
void adv_delete(struct adv_db *db);

// Transactions
struct adv_txn *adv_txn_begin(struct adv_db *db, int flags);
void adv_txn_commit(struct adv_db *db, struct adv_txn *txn);
void adv_txn_abort(struct adv_db *db, struct adv_txn *txn);

// Graph operations
node_p adv_node_add(struct adv_db *db);
int adv_edge_add(struct adv_db *db, node_p start, node_p end);
int adv_edge_remove(struct adv_db *db, node_p start, node_p end);

// Queries

// Debug functions
void adv_debug_print(struct adv_db *db);

#endif
