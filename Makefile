# Which C compiler should we use?
cc = clang

# TODO: how do I make this dynamic?
SOURCE = adv.c repl.c lib/linenoise/linenoise.c
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