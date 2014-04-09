#include <tests/minunit.h>
#include <src/db/store/btree.h>
#include <string.h>
#include <time.h>

static struct store *s = NULL;
static store_p root = 0;

char *test_open()
{
	s = store_open("testbtree", STORE_O_CREAT);
	mu_assert(s != NULL, "File should not be NULL");

	return NULL;
}

char *test_btree_alloc()
{
	root = btree_alloc(s);
	mu_assert(root != 0, "btree did not allocate root page");

	return NULL;
}

char *test_close()
{
	btree_free(s, root);
	store_close(s);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_btree_alloc);

	btree_debug_print(s, root, BTREE_DEBUG_FULL);
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
