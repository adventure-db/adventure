#include "hash.h"

#include <stdint.h>
#include <stdlib.h>

#include <lib/sds/sds.h>

static int default_compare(void *a, void *b)
{
	return sdscmp(a, b);
}

static unsigned long default_hash(void *a)
{
	size_t len = sdslen((sds)a);
	char *key = (sds)a;
	unsigned long hash = 0;
	unsigned long i = 0;

	for(hash = i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

struct hashmap *hmap_create(hash_fn hash, hash_compare compare)
{
	struct hashmap *map = calloc(1, sizeof(struct hashmap));
	map->hash = hash? hash : default_hash;
	map->compare = compare? compare : default_compare;

	return map;
}