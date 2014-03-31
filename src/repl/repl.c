#include "repl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <src/load.h>
#include <lib/linenoise/linenoise.h>

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_BLUE		"\x1b[34m"
#define ANSI_COLOR_MAGENTA	"\x1b[35m"
#define ANSI_COLOR_CYAN		"\x1b[36m"
#define ANSI_COLOR_RESET	"\x1b[0m"

#define SUCCESS_COLOR		"\x1b[0;32m"
#define ERROR_COLOR			"\x1b[0;31m"
 
void printHello()
{
	printf("%s\n%s\n\n", "Never fear quarrels, but seek hazardous adventures.", "- Alexandre Dumas");
}

void printGoodbye()
{
	printf("%s\n", "All great journeys must come to an end.");
}

void completion(const char *buf, linenoiseCompletions *lc)
{
	if(buf[0] == 'l') {
		linenoiseAddCompletion(lc, "load");
	}
	if(buf[0] == 's') {
		linenoiseAddCompletion(lc, "stats");
		linenoiseAddCompletion(lc, "status");
	}
}

int isExitCmd(const char *line)
{
	return 	!strcmp(line, "q") ||
			!strcmp(line, "quit") ||
			!strcmp(line, "exit");
}

// TODO: add register functions to make this cleaner
// TODO: make use of sds
int routecmd(char *cmd)
{
	char *tok = strtok(cmd, " ");

	if(!strcmp(tok, "status")) {
		printf("%s %s\n", SUCCESS_COLOR "Connected.", SUCCESS_COLOR "Ready." ANSI_COLOR_RESET);
	} else if(!strcmp(tok, "create")) {
		// Create a new database
		return 0;
	} else if(!strcmp(tok, "load")) {
		tok = strtok(NULL, " ");
		if(!tok) {
			printf(ERROR_COLOR "load requires a file path" ANSI_COLOR_RESET "\n");
			return 2;
		} else {
			// We have a file path
			printf("Loading %s...\n", tok);
			return load(tok);
		}
	} else if(!strcmp(tok, "gen")) {
		printf("We should generate a graph now!\n");
	} else {
		return -1;
	}
	return 0;
}

int repl(const char *prompt)
{
	printHello();
	linenoiseSetCompletionCallback(completion);

	//Test
	//generateGraph();

	char *line;
	while( (line = linenoise(prompt)) != NULL ) {
		if(line[0] == '\0') {
			continue;
		} else if (isExitCmd(line)) {
			break;
		} else {
			linenoiseHistoryAdd(line);
			if(routecmd(line) == -1) {
				printf(ANSI_COLOR_RED "%s is not a known command" ANSI_COLOR_RESET "\n", line);
			}
		}
		free(line);
	}

	printGoodbye();
	return 0;
}
