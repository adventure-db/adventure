#include "db.h"
#include "struct.h"
#include "store/edges.h"

#include <stdlib.h>

#include <lib/sds/sds.h>
#include <src/util/dbg.h>
#include <src/util/fs.h>

static inline struct dbf *adv_dbf_open(const char *dir, const char *file, int flags)
{
	sds path = sdscatprintf(sdsempty(), "%s/%s", dir, file);
	struct dbf *f = dbf_open(path, flags);
	sdsfree(path);
	return f;
}

//TODO: do some checking in herr
struct adv_db *adv_open(const char *dbname, int flags)
{
	int code = 0;

	// Create directory for the database
	sds dirname = sdscatprintf(sdsempty(), ".%s.db", dbname);
	code = fs_create_dir(dirname);
	check(code == 0, "Couldn't create new database on disk");

	struct adv_db *db = calloc(1, sizeof(struct adv_db));
	db->path = sdsnew(dbname);

	// Create and map the initial set of files
	// Nodes
	db->nodes = adv_dbf_open(dirname, "nodes", flags);
	check(db->nodes, "Couldn't create nodes file new database");

	// Edges
	db->edges = adv_dbf_open(dirname, "edges", flags);
	check(db->edges, "Couldn't create edges file for new database");

	sdsfree(dirname);
	return db;

error:
	sdsfree(dirname);
	return NULL;
}

void adv_close(struct adv_db *db)
{
	if(db) {
		dbf_close(db->nodes);
		dbf_close(db->edges);

		sdsfree(db->path);
		free(db);
	}
}

nodeid_t adv_node_add(struct adv_db *db)
{
	nodeid_t id = (db->nodes->end - 32) / 8;
	dbf_p p = dbf_alloc(db->nodes, sizeof(dbf_p));
	dbf_write_ui64(db->nodes, p, 0);
	return id;
}

int adv_edge_add(struct adv_db *db, nodeid_t start, nodeid_t end, uint64_t type)
{
	dbf_p e_old = dbf_read_ui64(db->nodes, 32 + 8 * start);
	dbf_p e_new = edges_add(db->edges, e_old, start, end, type);
	dbf_write_ui64(db->nodes, 32 + 8 * start, e_new);

	return 0;
}

int adv_edge_remove(struct adv_db *db, nodeid_t start, nodeid_t end)
{
	dbf_p e_old = dbf_read_ui64(db->nodes, 32 + 8 * start);
	dbf_p e_new = edges_remove(db->edges, e_old, start, end);
	dbf_write_ui64(db->nodes, 32 + 8 * start, e_new);

	return 0;
}

void adv_debug_print(struct adv_db *db)
{
	nodeid_t n_nodes = (db->nodes->end - 32) / 8;
	for(nodeid_t i=0; i<n_nodes; i++) {
		printf("Node %llu -> ", i);

		dbf_p e = dbf_read_ui64(db->nodes, 32 + 8*i);
		if(e != 0) {
			dbf_p size = dbf_read_ui64(db->edges, e);
			for(dbf_p p = e + 8; p < e + size; p += 8) {
				printf("%llu ", dbf_read_ui64(db->edges, p));
			}
		}
		printf("\n");
	}
}
