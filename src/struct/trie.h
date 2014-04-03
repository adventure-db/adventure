#ifndef ADV_TRIE_H
#define ADV_TRIE_H

#include <lib/sds/sds.h>

struct trie
{
	sds key;
	void *val;
	struct list *children;
};

struct trie *trie_create();
void trie_destroy(struct trie *trie);

int trie_add(struct trie *trie, const char *key, void *val);
void *trie_get(struct trie *trie, const char *key);
void trie_get_keys(struct trie *trie, const char *prefix, struct list *list);

#endif
