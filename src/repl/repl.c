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

static void hello()
{
	printf("%s\n%s\n\n", "Never fear quarrels, "
						 "but seek hazardous adventures.",
						 "~ Alexandre Dumas");
}

static void goodbye()
{
	printf("%s\n", "All great journeys must come to an end.");
}

static void completion(const char *buf, linenoiseCompletions *lc)
{
	struct list *list = list_create();
	trie_get_keys(cmds, buf, list);
	for (list_iter cur = list->first; cur != NULL; cur = cur->next) {
		linenoiseAddCompletion(lc, cur->val);
	}
	list_destroy(list);
}

static int repl_wants_exit(const char *line)
{
	return !strcmp(line, "q") ||
		   !strcmp(line, "quit") ||
		   !strcmp(line, "exit");
}

static int repl_route_cmd(char *line)
{
	char *cmd = strtok(line, " ");
	int (*fn)(char *) = (int (*)(char *)) trie_get(cmds, cmd);
	if(fn) return fn(strtok(NULL, "\0"));
	return -1;
}

void repl_add_cmd(char *str, int (*fn)(char *))
{
	trie_add(cmds, str, fn);
}

int repl(const char *prefix)
{
	cmds = trie_create();
	register_cmds();
	linenoiseSetCompletionCallback(completion);

	hello();
	char *line;
	sds prompt = sdsnew(prefix);

	while ((line = linenoise(prompt)) != NULL) {
		if (line[0] == '\0') {
			continue;
		} else if (repl_wants_exit(line)) {
			break;
		} else {
			linenoiseHistoryAdd(line);
			if (repl_route_cmd(line) == -1) {
				printf(ANSI_COLOR_RED "Unknown command: " ANSI_COLOR_RESET "%s\n", line);
			}
		}
		free(line);
	}
	goodbye();

	sdsfree(prompt);
	trie_destroy(cmds);
	return 0;
}
