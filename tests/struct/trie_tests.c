#include <tests/minunit.h>
#include <src/struct/trie.h>
#include <src/struct/list.h>

static struct trie *trie = NULL;
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
	trie = trie_create();
	mu_assert(trie != NULL, "Trie should not be null");

	return NULL;
}

char *test_destroy()
{
	trie_destroy(trie);

	return NULL;
}

char *test_add_get()
{
	trie_add(trie, "banana", val1);
	trie_add(trie, "band", val2);
	trie_add(trie, "green", "nice");

	void *elem = trie_get(trie, "green");
	mu_assert(elem == "nice", "Key green should have value 'nice'");

	elem = trie_get(trie, "banana");
	mu_assert(elem == val1, "Key banana should have value 'test1 data'");

	elem = trie_get(trie, "band");
	mu_assert(elem == val2, "Key band should have value 'test2 data'");

	elem = trie_get(trie, "turquoise");
	mu_assert(elem == NULL, "Key turquoise should return a NULL element");

	struct list *list = list_create();
	trie_get_keys(trie, "ba", list);
	mu_assert(list_count(list) == 2, "There should be 2 keys beginning with 'ba'");

	char *key = list_pop(list);
	key = list_pop(list);
	list_destroy(list);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_add_get);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
