#include "adv.h"
#include "store/nodes.h"
#include "store/edges.h"

#include <stdlib.h>

#include <lib/sds/sds.h>
#include <src/util/dbg.h>
#include <src/util/fs.h>

static inline struct store *adv_dbf_open(const char *dir, const char *file, int flags)
{
	sds path = sdscatprintf(sdsempty(), "%s/%s", dir, file);
	struct store *f = store_open(path, flags);
	sdsfree(path);
	return f;
}

//TODO: do some checking in herr
struct adv_db *adv_open(const char *dbname, int flags)
{
	int code = 0;

	// Create directory for the database
	sds dirname = sdscatprintf(sdsempty(), "%s.db", dbname);
	if(flags & ADV_CREATE) {
		code = fs_create_dir(dirname);
		check(code == 0, "Couldn't create new database on disk");
	}

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
		store_close(db->nodes);
		store_close(db->edges);

		sdsfree(db->path);
		free(db);
	}
}

int adv_repair(struct adv_db *db)
{
	store_repair(db->nodes);
	store_repair(db->edges);
	return 0;
}

node_p adv_node_add(struct adv_db *db)
{
	return nodes_add(db->nodes);
}

int adv_edge_add(struct adv_db *db, node_p start, node_p end)
{
	edge_p prev = nodes_edges_from(db->nodes, start);
	edge_p next = edges_add(db->edges, prev, start, end);
	nodes_edges_set(db->nodes, start, next);

	return 0;
}

int adv_edge_remove(struct adv_db *db, node_p start, node_p end)
{
	edge_p prev = nodes_edges_from(db->nodes, start);
	edge_p next = edges_remove(db->edges, prev, start, end);
	nodes_edges_set(db->nodes, start, next);

	return 0;
}

void adv_debug_print(struct adv_db *db)
{
	for(node_p i = 0; i < nodes_count(db->nodes); i++) {
		printf("%llu -> ", i);

		
		edge_p e = nodes_edges_from(db->nodes, i);

		if(e != 0) {
			edge_p size = store_read_ui64(db->edges, e);
			for(edge_p p = e + 8; p < e + size; p += 8) {
				printf("%llu ", store_read_ui64(db->edges, p));
			}
		}

		/*
		while(e = edges_next(db->edges, e)) {
			printf("%llu ", edges_node_to(db->edges, e));
		}
		*/

		printf("\n");
	}
}
