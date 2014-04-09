A tiny fast graph database engine written in C

Design goals:
- Embeddable core
- ACID transactions with snapshot isolation
- Fast, lock-free reads and traversals (single writer)
- Efficient on supernodes

Components:
- Data structures
- Algorithms
- Query analyzer

SOURCE DIRS
db - contains core database logic
struct - commonly used data structures
	list - doubly-linked list
	darray - dynamic array
	hashmap - hash table (chaining)

util - utility functions for debugging, etc...
repl - code for interactive command line

EXTERNAL LIBRARIES
jsmn - tiny JSON tokenizer (MIT)
linenoise - line editing from terminal, used for REPL (BSD)
sds - simple dynamic string library (BSD)

Future?
check - unit testing (LGPL)
libcurl - networking and data transfer (MIT/X)
