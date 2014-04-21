#include <tests/minunit.h>
#include <src/db/store/btree.h>
#include <string.h>
#include <time.h>

static struct store *s = NULL;
static page_p root;

static int n = 10000;

/*
	Big TODOs:
	1. Duplicates
	2. Variable length keys (+values)
	3. Removals
*/

char *test_open()
{
	s = store_open("btree.test", STORE_O_CREAT);
	store_resize(s, 1024 * 1024 * 256);
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
	btree_destroy(s, root);
	store_close(s);

	return NULL;
}

// Perform timed batch insertions in sequential order
char *test_perf_insert_seq()
{
	srand(time(NULL));

	clock_t start = clock(), diff;
	for (int i=0; i<n; i++) {
		int item = i;
		root = btree_add(s, root, item, item);
	}
	diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	double ops = n * 1000;
	ops /= msec;
	printf("Time to insert %u items: %u ms\n", n, msec);
	printf("Time per operation: %u us\n", (1000*msec)/n);
	printf("Operations per second: %f\n", ops);

	return NULL;
}

// Perform timed batch queries in random order
char *test_perf_query_rand()
{
	srand(time(NULL));

	clock_t start = clock(), diff;
	struct bt_cur cur;
	for (int i=0; i<n; i++) {
		int item = rand();
		cur = btree_find(s, root, item);
	}
	diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	double ops = n * 1000;
	ops /= msec;
	printf("Time to query %u items: %u ms\n", n, msec);
	printf("Time per operation: %u us\n", (1000*msec)/n);
	printf("Operations per second: %f\n", ops);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_open);
	mu_run_test(test_btree_create);

	// Performance tests
	mu_run_test(test_perf_insert_seq);
	//mu_run_test(test_perf_query_rand);

	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
