#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <src/repl/repl.h>

int main(int argc, char *argv[]) {
	char *programName = argv[0];

	// Parse command line options
	while (argc > 1) {
		argc--, argv++;
		if (!strcmp(*argv, "-help")) {
			printf("Fear not, help is coming soon.\n");
			return 0;
		} else {
			fprintf(stderr, "adventure, a tiny graph database.\n\n");
			fprintf(stderr, "Usage: %s [-help]\n", programName);
			return 1;
		}
	}

	// Provide a REPL
	return repl("o-> ");
}
