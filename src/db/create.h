#ifndef ADV_CREATE_H
#define ADV_CREATE_H

#include <lib/sds/sds.h>

// Default files to 256MB each
#define DEF_NODE_FILE_SIZE (1024 * 1024 * 256)
#define DEF_EDGE_FILE_SIZE (1024 * 1024 * 256)

struct adv_db
{
	sds name;
	sds path;

	// Pointers to memory mapped files that contain the actual data
	struct node **nodes;
	struct edge **edges;
	void **kv;
};

// Create a new database
int adv_create_db(const char *dbname);

// Load/close an existing database from disk
struct adv_db *adv_load_db(const char *dbname);
void adv_close_db(struct adv_db *db);

#endif
