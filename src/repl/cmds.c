#include "cmds.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <src/repl/load.h>
#include <src/repl/repl.h>
#include <src/db/db.h>

static struct adv_db *db = NULL;

int load_json(char *line)
{
	char *tok = strtok(line, " ");
	if(!tok) {
		printf(ERROR_COLOR "load requires a file path" ANSI_COLOR_RESET "\n");
		return 2;
	} else {
		// We have a file path
		printf("Loading %s...\n", tok);
		return load(tok);
	}
}

int status(char *line)
{
	printf("%s %s\n", SUCCESS_COLOR "Connected.", SUCCESS_COLOR "Ready." ANSI_COLOR_RESET);
	return 0;
}

int sting(char *line)
{
	printf("%s\n", ANSI_COLOR_RED "Ouch." ANSI_COLOR_RESET);
	return 0;
}

int open_db(char *line)
{
	char *tok = strtok(line, " ");
	if(!tok) {
		printf(ERROR_COLOR "open requires a database name" ANSI_COLOR_RESET "\n");
		return 2;
	} else {
		db = adv_open(tok, 0);
		if(!db) printf("Error opening database");
		return 0;
	}
}

int create_db(char *line)
{
	char *tok = strtok(line, " ");
	if(!tok) {
		printf(ERROR_COLOR "create requires a database name" ANSI_COLOR_RESET "\n");
		return 2;
	} else {
		// We have a file path
		db = adv_open(tok, ADV_CREATE);
		if(!db) printf("Error creating database");
		return 0;
	}
}

int add_node(char *line)
{
	if(db) {
		adv_node_add(db);
		return 0;
	}
	return -1;
}

int add_edge(char *line)
{
	if(db) {
		char *tok = strtok(line, " ");
		nodeid_t start = atoi(tok);
		tok = strtok(NULL, " ");
		nodeid_t end = atoi(tok);

		adv_edge_add(db, start, end, 0);
		return 0;
	}
	return -1;
}

int remove_edge(char *line)
{
	if(db) {
		char *tok = strtok(line, " ");
		nodeid_t start = atoi(tok);
		tok = strtok(NULL, " ");
		nodeid_t end = atoi(tok);

		adv_edge_remove(db, start, end);
		return 0;
	}
	return -1;
}

int stress(char *line)
{
	int n = 20;
	int m = 8;

	for(int i=0; i<n; i++) {
		adv_node_add(db);
	}
	printf("Added %d nodes\n", n);

	for(int j=0; j<m; j++) {
		adv_edge_add(db, 1, j+2, 0);
	}
	printf("Added %d edges from node 1", m);
	return 0;
}

int debug(char *line)
{
	if(db) adv_debug_print(db);
	else printf("No database open. C'mon!\n");
	return 0;
}

void register_cmds()
{
	repl_add_cmd("load", load_json);
	repl_add_cmd("status", status);
	repl_add_cmd("stats", status);
	repl_add_cmd("sting!", sting);

	// Database commands
	repl_add_cmd("create", create_db);
	repl_add_cmd("open", open_db);
	repl_add_cmd("+n", add_node);
	repl_add_cmd("+e", add_edge);
	repl_add_cmd("-e", remove_edge);
	repl_add_cmd("stress", stress);
	repl_add_cmd("debug", debug);
}
