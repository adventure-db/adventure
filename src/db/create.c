#include "create.h"
#include "struct.h"

#include <stdlib.h>

#include <lib/sds/sds.h>
#include <src/util/dbg.h>
#include <src/util/fs.h>

static int adv_create_file(const char *filename, unsigned long size)
{
	return fs_create_file(filename, size);
}

//TODO: do some checking in herr
int adv_create_db(const char *dbname)
{
	int code = 0;
	sds filename = sdsempty();
	sds dirname = sdsempty();

	// Create directory for the database
	dirname = sdscatprintf(dirname, ".%s.db", dbname);
	code = fs_create_dir(dirname);
	check_debug(code == 0, "Couldn't create dir");

	// Create and map the initial set of files
	filename = sdscatprintf(filename, "%s/nodes.%i", dirname, 0);
	code = adv_create_file(filename, DEF_NODE_FILE_SIZE);
	check_debug(code == 0, "Couldn't create nodes file");

error:
	sdsfree(filename);
	sdsfree(dirname);
	return code;
}

struct adv_db *adv_load_db(const char *dbname)
{
	struct adv_db *db = calloc(1, sizeof(struct adv_db));
	/*
		1) Check if directory exists
		2) Set up adv_db struct with metadata
		3) Map all nodes, edges, and kv files into memory
	*/
	return db;
}

void adv_close_db(struct adv_db *db)
{
	if(db) {
		sdsfree(db->name);
		sdsfree(db->path);
		free(db);
	}
}
