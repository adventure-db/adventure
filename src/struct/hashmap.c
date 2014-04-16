#include "hashmap.h"
#include "list.h"

#include <stdint.h>
#include <stdlib.h>

#include <src/util/dbg.h>

#define DEFAULT_SIZE 300

static int default_compare(const void *a, const void *b)
{
	return strcmp( (const char *) a, (const char *) b );
}

static hash_t default_hash(const void *a)
{
	size_t len = strlen((const char *)a);
	const char *key = (const char *)a;

	hash_t hash = 0;
	hash_t i = 0;

	for (hash = i = 0; i < len; ++i)
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
	map->buckets = calloc(DEFAULT_SIZE, sizeof(struct list *));
	map->size = DEFAULT_SIZE;
	check_mem(map->buckets);

	return map;

error:
	hmap_destroy(map);
	return NULL;
}

void hmap_destroy(struct hashmap *map)
{
	if (map) {
		if (map->buckets) {
			for (int i=0; i<map->size; i++) {
				struct list *bucket = map->buckets[i];
				if (bucket) {
					list_clear(bucket);
					list_destroy(bucket);
				}
			}
			free(map->buckets);
		}
		free(map);
	}
}

static void hmap_rehash(struct hashmap *in, struct hashmap *out)
{
	for (int i=0; i<in->size; i++) {
		struct list *bucket = in->buckets[i];
		if (bucket) {
			struct listelem *cur;
			for (cur = bucket->first; cur != NULL; cur = cur->next) {
				struct hashentry *entry = (struct hashentry *) cur->val;
				if (entry) hmap_insert(out, entry->key, entry->val);
			}
		}
	}
}

void hmap_resize(struct hashmap *map, size_t size)
{
	check(map, "Hashmap not initialized");

	struct hashmap *temp = calloc(1, sizeof(struct hashmap));
	temp->buckets = map->buckets;
	temp->size = map->size;
	temp->hash = map->hash;
	temp->compare = map->compare;

	struct list **buckets = calloc(size, sizeof(struct list *));
	map->buckets = buckets;
	map->size = size;

	hmap_rehash(temp, map);
	hmap_destroy(temp);

error:
	return;
}

static struct hashentry *hmap_make_entry(void *key, void *val, hash_t hash)
{
	struct hashentry *entry = calloc(1, sizeof(struct hashentry));
	check_mem(entry);

	entry->key = key;
	entry->val = val;
	entry->hash = hash;

	return entry;

error:
	return NULL;
}

static struct listelem *hmap_get_entry(struct list *bucket, void *key, hash_t hash, hash_compare compare)
{
	check(bucket, "Bucket is empty");

	struct listelem *cur;
	struct hashentry *entry;
	for (cur = bucket->first; cur != NULL; cur = cur->next) {
		entry = (struct hashentry *) cur->val;
		if (entry && entry->hash == hash && compare(entry->key, key) == 0) {
			return cur;
		}
	}

error:
	return NULL;
}

static struct list *hmap_get_bucket(struct hashmap *map, void *key, hash_t index)
{
	struct list *bucket = map->buckets[index];
	if (!bucket) {
		bucket = list_create();
		check(bucket, "Could not create new bucket");
		map->buckets[index] = bucket;
	}
	return bucket;

error:
	return NULL;
}

void hmap_insert(struct hashmap *map, void *key, void *val)
{
	hash_t hash = map->hash(key);

	struct list *bucket = hmap_get_bucket(map, key, hash % map->size);
	check(bucket, "Error getting bucket");

	struct hashentry *entry = hmap_make_entry(key, val, hash);
	check(entry, "Could not create a new hashmap entry");
	list_unshift(bucket, entry);

error:
	return;
}

void *hmap_remove(struct hashmap *map, void *key)
{
	hash_t hash = map->hash(key);
	struct list *bucket = map->buckets[ hash % map->size ];
	check(bucket, "Bucket doesn't exist");

	struct listelem *elem = hmap_get_entry(bucket, key, hash, map->compare);
	check(elem, "Could not get entry");

	struct hashentry *entry = (struct hashentry *) elem->val;
	void *val = entry->val;
	free(entry);
	list_remove(bucket, elem);
	return val;

error:
	return NULL;
}

bool hmap_contains(struct hashmap *map, void *key)
{
	hash_t hash = map->hash(key);
	struct list *bucket = map->buckets[ hash % map->size ];
	if (!bucket) return false;

	struct listelem *elem = hmap_get_entry(bucket, key, hash, map->compare);
	return elem != NULL;
}

void hmap_set(struct hashmap *map, void *key, void *val)
{
	hash_t hash = map->hash(key);

	struct list *bucket = hmap_get_bucket(map, key, hash % map->size);
	check(bucket, "Error getting bucket");

	struct listelem *elem = hmap_get_entry(bucket, key, hash, map->compare);
	struct hashentry *entry;

	if (elem) {
		entry = (struct hashentry *) elem->val;
		check(entry, "Could not find or create a new hashmap entry");
		entry->val = val;
	} else {
		entry = hmap_make_entry(key, val, hash);
		check(entry, "Could not find or create a new hashmap entry");
		list_unshift(bucket, entry);
	}

error:
	return;
}

void *hmap_get(struct hashmap *map, void *key)
{
	hash_t hash = map->hash(key);
	struct list *bucket = map->buckets[ hash % map->size ];
	if (!bucket) return NULL;

	struct listelem *elem = hmap_get_entry(bucket, key, hash, map->compare);
	if (!elem) return NULL;

	struct hashentry *entry = (struct hashentry *) elem->val;
	return entry->val;
}
