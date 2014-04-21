#include <tests/minunit.h>
#include <src/db/store/btree.h>
#include <string.h>
#include <time.h>

static struct store *s = NULL;
static page_p root;

/*
	Big TODOs:
	1. Duplicates
	2. Variable length keys (+values)
	3. Removals
*/

char *test_open()
{
	s = store_open("btree.test", STORE_O_CREAT);
	mu_assert(s != NULL, "File should not be NULL");

	return NULL;
}

char *test_btree_create()
{
	root = btree_create(s);
	mu_assert(root != 0, "btree did not allocate root page");

	return NULL;
}

char *test_close()
{
	//btree_destroy(s, root);
	store_close(s);

	return NULL;
}

// Insert sequential keys
char *test_insert_seq()
{
	for (int i=0; i<10; i++) {
		root = btree_add(s, root, i, i);
	}

	return NULL;
}

// Insert with duplicates
char *test_insert_dup()
{
	for (int i=0; i<20; i++) {
		int item = 1;
		root = btree_add(s, root, item, i);
	}

	return NULL;
}

// Simple query
char *test_query_basic()
{
	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_btree_create);

	// Basic tests
	mu_run_test(test_insert_seq);
	//mu_run_test(test_insert_dup);
	mu_run_test(test_query_basic);
	
	btree_debug_print(s, root, BTREE_DEBUG_FULL);
	
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
