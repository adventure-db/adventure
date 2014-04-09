# Which C compiler should we use?
CC = clang

OUTPUT = bin/adv

# Sources
SOURCES := $(wildcard src/**/*.c src/**/**/*.c src/*.c lib/linenoise/linenoise.c lib/sds/sds.c)
SOURCES := $(filter-out src/main.c, $(SOURCES))
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

# Includes
LIBRARIES = lib/jsmn/libjsmn.a
HEADERS = $(wildcard lib/** lib)
HEADERS_DIRS = -I$(CURDIR)

# Tests
TEST_SRC = $(wildcard tests/**/*_tests.c tests/*_tests.c)
TESTS = $(patsubst %.c, %, $(TEST_SRC))

# Compiler flags
CFLAGS = -Wall -Werror -g -v $(HEADERS_DIRS) $(SOURCES)
LDFLAGS = $(LIBRARIES)

default:
	@mkdir -p bin
	@mkdir -p build
	$(CC) $(CFLAGS) src/main.c $(LDFLAGS) -o $(OUTPUT)

valgrind:
	VALGRIND = "valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# Testing
.PHONY: tests
tests: $(TESTS)
	sh ./tests/runtests.sh

test: $(TESTS)

# Clean up output
.PHONY: clean
clean:
	rm -f *.o *~ tests/*.log $(OUTPUT) $(TESTS) testfile_*
	rm -rf `find . -name "*.dSYM" -print`

# Check for dangerous functions
BADFUNCS = '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) || true
