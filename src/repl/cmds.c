#include "cmds.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <src/repl/load.h>
#include <src/db/create.h>

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

int create_db(char *line)
{
	char *tok = strtok(line, " ");
	if(!tok) {
		printf(ERROR_COLOR "create requires a database name" ANSI_COLOR_RESET "\n");
		return 2;
	} else {
		// We have a file path
		int val = adv_create_db(tok);
		if(val != 0) printf("Error creating database, err = %i\n", val);
		return 0;
	}
}
