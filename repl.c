#include "repl.h"

void printHello()
{
	printf("%42s", "ADVENTURE v0.0.1\n\n");
	printf(
	"         _---~~~~--__                      __--~~~~---__\n"
	"        `\\---~~~~~~~~\\                    //~~~~~~~~---/'  \n"
	"          \\/~~~~~~~~~\\||                  ||/~~~~~~~~~\\/ \n"
	"                      `\\                //'\n"
	"                        `\\            //'\n"
	"                          ||          ||      \n"
	"                ______--~~~~~~~~~~~~~~~~~~--______\n"
	"           ___ // _-~                        ~-_ \\ ___\n"
	"          `\\__)\\/~                              ~\\/(__/'\n"
	"           _--`-___                            ___-'--_\n"
	"         /~     `\\ ~~~~~~~~------------~~~~~~~~ /'     ~\\\n"
	"        /|        `\\         ________         /'        |\\\n"
	"       | `\\   ______`\\_      \\------/      _/'______   /' |\n"
	"       |   `\\_~-_____\\ ~-________________-~ /_____-~_/'   |\n"
	"       `.     ~-__________________________________-~     .'\n"
	"        `.      [_______/------|~~|------\\_______]      .'\n"
	"         `\\--___((____)(________\\/________)(____))___--/'\n"
	"          |>>>>>>||                            ||<<<<<<|\n"
	"          `\\<<<<</'                            `\\>>>>>/' \n\n");

	printf("%60s", "Buckle your seatbelts. We're going on an adventure.\n\n");
}

void printGoodbye()
{
	printf("%52s", "All great journeys must come to an end.\n");
}

void completion(const char *buf, linenoiseCompletions *lc) {
	if (buf[0] == 's') {
		linenoiseAddCompletion(lc, "stats");
		linenoiseAddCompletion(lc, "status");
	}
}

int repl(const char *prompt)
{
	printHello();
    linenoiseSetCompletionCallback(completion);

	char *line;
	while( (line = linenoise(prompt)) != NULL ) {
		if( !strncmp(line, "exit", 4) ) {
			printGoodbye();
			break;
		} else {
			printf("You wrote %s\n", line);
            linenoiseHistoryAdd(line);
		}
		free(line);
	}
	return 1;
}