#include "cmds.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <src/repl/load.h>
#include <src/repl/repl.h>
#include <src/db/adv.h>

static struct adv_db *db = NULL;

static int load_json(char *line)
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

static int status(char *line)
{
	printf("%s %s\n", SUCCESS_COLOR "Connected.", SUCCESS_COLOR "Ready." ANSI_COLOR_RESET);
	return 0;
}

static int sting(char *line)
{
	printf("%s\n", ANSI_COLOR_RED "Ouch." ANSI_COLOR_RESET);
	return 0;
}

static int open_db(char *line)
{
	char *tok = strtok(line, " ");
	if(!tok) {
		printf(ERROR_COLOR "open requires a database name" ANSI_COLOR_RESET "\n");
		return 2;
	} else {
		db = adv_open(tok, 0);
		if(!db) printf("Error opening database\n");
		return 0;
	}
}

static int close_db(char *line)
{
	if(db) adv_close(db);
	else printf("No database is open.\n");
	return 0;
}

static int create_db(char *line)
{
	char *tok = strtok(line, " ");
	if(!tok) {
		printf(ERROR_COLOR "create requires a database name" ANSI_COLOR_RESET "\n");
		return 2;
	} else {
		// We have a file path
		db = adv_open(tok, ADV_CREATE);
		if(!db) printf("Error creating database\n");
		return 0;
	}
}

static int add_node(char *line)
{
	if(db) {
		adv_node_add(db);
		return 0;
	}
	return -1;
}

static int add_edge(char *line)
{
	if(db) {
		char *tok = strtok(line, " ");
		node_p start = atoi(tok);
		tok = strtok(NULL, " ");
		node_p end = atoi(tok);

		adv_edge_add(db, start, end);
		return 0;
	}
	return -1;
}

static int remove_edge(char *line)
{
	if(db) {
		char *tok = strtok(line, " ");
		node_p start = atoi(tok);
		tok = strtok(NULL, " ");
		node_p end = atoi(tok);

		adv_edge_remove(db, start, end);
		return 0;
	}
	return -1;
}

static int debug_print(char *line)
{
	if(db) adv_debug_print(db);
	else printf("No database currently open.\n");
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
	repl_add_cmd("close", close_db);
	repl_add_cmd("+n", add_node);
	repl_add_cmd("+e", add_edge);
	repl_add_cmd("-e", remove_edge);
	repl_add_cmd("debug", debug_print);
}
