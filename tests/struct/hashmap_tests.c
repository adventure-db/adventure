#include <tests/minunit.h>
#include <src/struct/hashmap.h>

static struct hashmap *map = NULL;
char *key1 = "test1";
char *key2 = "test2";
char *key3 = "test3";
char *key4 = "test4";

char *val1 = "test1 data";
char *val2 = "test2 data";
char *val3 = "test3 data";
char *val4 = "howdy";

char *test_create()
{
	map = hmap_create(NULL, NULL);
	mu_assert(map != NULL, "Map should not be null");
	mu_assert(map->size > 0, "Map size should be > 0");
	mu_assert(map->buckets, "Map buckets should be allocated");

	return NULL;
}

char *test_destroy()
{
	hmap_destroy(map);

	return NULL;
}

char *test_basics()
{
	// Test insertion
	hmap_insert(map, key1, val1);
	hmap_insert(map, key2, val2);
	hmap_insert(map, key3, val3);

	// Test get
	mu_assert(hmap_get(map, "bleep") == NULL, "Elem should be NULL (non-existent key) on get");
	mu_assert(hmap_get(map, key1) == val1, "Key1 != val1 on get");
	mu_assert(hmap_get(map, key2) == val2, "Key2 != val2 on get");
	mu_assert(hmap_get(map, key3) == val3, "Key3 != val3 on get");
	mu_assert(hmap_get(map, "bloop") == NULL, "Elem should be NULL (non-existent key) on get");

	// Test set
	hmap_set(map, key4, val4);
	hmap_set(map, key1, val2);
	hmap_set(map, key2, val3);
	hmap_set(map, key3, val1);

	// Test get again
	mu_assert(hmap_get(map, key4) == val4, "Key4 != val4 on get");
	mu_assert(hmap_get(map, key1) == val2, "Key1 != val2 on get");
	mu_assert(hmap_get(map, key2) == val3, "Key2 != val2 on get");
	mu_assert(hmap_get(map, key3) == val1, "Key3 != val3 on get");

	hmap_set(map, key1, val1);
	hmap_set(map, key2, val2);
	hmap_set(map, key3, val3);

	// Test removal
	char *elem = (char *) hmap_remove(map, key1);
	mu_assert(elem == val1, "Key1 != val1");

	elem = (char *) hmap_remove(map, "non-existent key");
	mu_assert(!elem, "Elem should be NULL (non-existent key)");

	elem = (char *) hmap_remove(map, key2);
	mu_assert(elem == val2, "Key2 != val2");

	elem = (char *) hmap_remove(map, key3);
	mu_assert(elem == val3, "Key3 != val3");

	elem = (char *) hmap_remove(map, "bloop");
	mu_assert(!elem, "Elem should be NULL (non-existent key)");

	return NULL;
}

char *test_resize()
{
	// Test set
	hmap_set(map, key4, val4);
	hmap_set(map, key1, val2);
	hmap_set(map, key2, val3);
	hmap_set(map, key3, val1);

	hmap_resize(map, 1024);
	mu_assert(map->size == 1024, "Map size != 1024 on resize");

	// Test get
	mu_assert(hmap_get(map, key4) == val4, "Key4 != val4 on get");
	mu_assert(hmap_get(map, key1) == val2, "Key1 != val2 on get");
	mu_assert(hmap_get(map, key2) == val3, "Key2 != val2 on get");
	mu_assert(hmap_get(map, key3) == val1, "Key3 != val3 on get");

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_basics);
	mu_run_test(test_resize);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
