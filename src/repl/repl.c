#include "repl.h"
#include "macros.h"
#include "cmds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <src/repl/load.h>
#include <src/struct/trie.h>
#include <src/struct/list.h>
#include <lib/linenoise/linenoise.h>

static struct trie *cmds = NULL;

void hello()
{
	printf("%s\n%s\n\n", "Never fear quarrels, but seek hazardous adventures.", "- Alexandre Dumas");
}

void goodbye()
{
	printf("%s\n", "All great journeys must come to an end.");
}

void completion(const char *buf, linenoiseCompletions *lc)
{
	struct list *list = list_create();
	trie_get_keys(cmds, buf, list);
	for(list_iter cur = list->first; cur != NULL; cur = cur->next) {
		linenoiseAddCompletion(lc, cur->val);
	}
	list_destroy(list);
}

int repl_wants_exit(const char *line)
{
	return 	!strcmp(line, "q") ||
			!strcmp(line, "quit") ||
			!strcmp(line, "exit");
}

void repl_add_cmd(char *str, int (*fn)(char *))
{
	trie_add(cmds, str, fn);
}

int repl_route_cmd(char *line)
{
	char *cmd = strtok(line, " ");
	int (*fn)(char *) = (int (*)(char *)) trie_get(cmds, cmd);
	if(fn) return fn( strtok(NULL, "\0") );
	return -1;
}

static void register_cmds()
{
	repl_add_cmd("create", create_db);
	repl_add_cmd("load", load_json);
	repl_add_cmd("status", status);
	repl_add_cmd("stats", status);
	repl_add_cmd("sting!", sting);
}

int repl(const char *prompt)
{
	cmds = trie_create();
	register_cmds();
	linenoiseSetCompletionCallback(completion);

	hello();
	char *line;
	while( (line = linenoise(prompt)) != NULL ) {
		if(line[0] == '\0') {
			continue;
		} else if(repl_wants_exit(line)) {
			break;
		} else {
			linenoiseHistoryAdd(line);
			if(repl_route_cmd(line) == -1) {
				printf(ANSI_COLOR_RED "%s is not a known command" ANSI_COLOR_RESET "\n", line);
			}
		}
		free(line);
	}
	goodbye();

	trie_destroy(cmds);
	return 0;
}
