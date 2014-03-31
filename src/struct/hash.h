#ifndef ADV_HASH_H
#define ADV_HASH_H

typedef int (*hash_compare)(void *a, void *b);
typedef unsigned long (*hash_fn)(void *key);

struct hashmap
{
	struct list *buckets;
	unsigned long n;
	unsigned long size;

	hash_compare compare;
	hash_fn hash;
};

struct hashentry
{
	void *key;
	void *val;
	unsigned long hash;
};

struct hashmap *hmap_create(hash_fn, hash_compare);
void hmap_destroy(struct hashmap *map);

void hmap_set(struct hashmap *map, void *key, void *val);
void *hmap_get(struct hashmap *map, void *key);
void *hmap_remove(struct hashmap *map, void *key);

#endif
