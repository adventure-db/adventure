# Which C compiler should we use?
cc = clang

# TODO: how do I make this dynamic?
SOURCE = adv.c wmmap.c graph.c repl.c load.c lib/linenoise/linenoise.c lib/jsmn/jsmn.c
OUTPUT = adv

# Includes
LIBRARIES = 
HEADERS =

# Compiler flags
CFLAGS = -Wall -Werror -g -v $(HEADERS) $(SOURCE)
LDFLAGS = $(LIBRARIES)

all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OUTPUT)

clean:
	rm -f *.o *~ $(OUTPUT)