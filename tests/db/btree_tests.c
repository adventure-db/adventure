#include <tests/minunit.h>
#include <src/db/store/btree.h>
#include <string.h>
#include <time.h>

static struct bt_page root;
static struct store *s = NULL;

char *test_open()
{
	s = store_open("testbtree", STORE_O_CREAT);
	mu_assert(s != NULL, "File should not be NULL");

	return NULL;
}

char *test_btree_create()
{
	root = btree_create(s);
	mu_assert(root.ptr != 0, "btree did not allocate root page");

	return NULL;
}

char *test_close()
{
	btree_destroy(root);
	store_close(s);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_btree_create);

	btree_debug_print(root, BTREE_DEBUG_FULL);
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
