#include "trie.h"
#include "list.h"

#include <src/util/dbg.h>

struct trie *trie_create()
{
	struct trie *trie = calloc(1, sizeof(struct trie));
	check_mem(trie);
	return trie;

error:
	return NULL;
}

void trie_destroy(struct trie *trie)
{
	if(trie) {
		sdsfree(trie->key);

		if(trie->children) {
			struct listelem *cur;
			for(cur = trie->children->first; cur != NULL; cur = cur->next) {
				trie_destroy(cur->val);
			}
			list_destroy(trie->children);
		}
		free(trie);
	}
}

static struct trie *trie_make_node(const char *key, int len, void *val)
{
	struct trie *trie = trie_create();
	trie->key = sdsnewlen(key, len);
	trie->val = val;
	return trie;
}

int trie_add(struct trie *trie, const char *key, void *val)
{
	if(!trie) return -1;

	int len = trie->key? sdslen(trie->key) : 0;
	if(strncmp(trie->key, key, len) != 0) return -1;
	if(len == strlen(key)) return 0;

	if(trie->children) {
		struct listelem *cur;
		for(cur = trie->children->first; cur != NULL; cur = cur->next) {
			int res = trie_add(cur->val, key, val);
			if(res == 0) return res;
		}
	} else {
		trie->children = list_create();
	}

	// Insert a node here
	struct trie *node = trie_make_node(key, len + 1, val);
	list_push(trie->children, node);
	return trie_add(node, key, val);
}

static struct trie *trie_get_subtrie(struct trie *trie, const char *key)
{
	if(!trie) return NULL;

	int len = trie->key? sdslen(trie->key) : 0;
	if(strncmp(trie->key, key, len) != 0) return NULL;

	if(!trie->children) {
		return len == strlen(key)? trie : NULL;
	}

	struct listelem *cur;
	for(cur = trie->children->first; cur != NULL; cur = cur->next) {
		struct trie *res = trie_get_subtrie(cur->val, key);
		if(res) return res;
	}
	return NULL;
}

void *trie_get(struct trie *trie, const char *key)
{
	struct trie *res = trie_get_subtrie(trie, key);
	if(!res) return NULL;
	return res->val;
}

void trie_get_keys(struct trie *trie, const char *prefix, struct list *list)
{
	if(!trie) return;

	int len = trie->key? sdslen(trie->key) : 0;
	if(len <= strlen(prefix) && strncmp(trie->key, prefix, len) != 0) return;

	if(trie->children) {
		struct listelem *cur;
		for(cur = trie->children->first; cur != NULL; cur = cur->next) {
			trie_get_keys(cur->val, prefix, list);
		}
	} else {
		list_push(list, trie->key);
	}
}
