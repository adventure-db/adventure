#ifndef ADV_HASHMAP_H
#define ADV_HASHMAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef unsigned int hash_t;
typedef int (*hash_compare)(const void *a, const void *b);
typedef hash_t (*hash_fn)(const void *key);

struct hashmap
{
	struct list **buckets;
	size_t size;

	hash_compare compare;
	hash_fn hash;
};

struct hashentry
{
	void *key;
	void *val;
	hash_t hash;
};

struct hashmap *hmap_create(hash_fn hash, hash_compare compare);
void hmap_destroy(struct hashmap *map);

void hmap_resize(struct hashmap *map, size_t size);

void hmap_insert(struct hashmap *map, void *key, void *val);
void *hmap_remove(struct hashmap *map, void *key);
bool hmap_contains(struct hashmap *map, void *key);

void hmap_set(struct hashmap *map, void *key, void *val);
void *hmap_get(struct hashmap *map, void *key);

#endif
