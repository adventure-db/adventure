#include <tests/minunit.h>
#include <src/db/store/btree.h>
#include <string.h>
#include <time.h>

static struct store *s = NULL;
static page_p root;
static struct cursor cur;

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
	cur = btree_cursor(s, root);
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
char *test_cur_insert()
{
	btree_cursor_add(&cur, 1, 1);
	btree_cursor_add(&cur, 2, 2);
	//btree_cursor_add(&cur, 3, 3);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_btree_create);

	// Basic tests
	mu_run_test(test_cur_insert);

	printf("\n");
	cursor_debug(&cur);
	btree_debug(s, cur.root, BTREE_DEBUG_FULL);
	printf("\n");
	
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
