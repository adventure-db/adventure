#include <tests/minunit.h>
#include <src/db/store/btree.h>
#include <string.h>
#include <time.h>

static struct store *s = NULL;
static page_p root;

static int n = 10;

char *test_open()
{
	s = store_open("testbtree", STORE_O_CREAT);
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

char *test_insert_dup()
{
	for(int i=0; i<n; i++) {
		int item = 1;
		root = btree_add(s, root, item, i);
	}

	return NULL;
}

char *test_query_basic()
{
	return NULL;
}

char *test_perf_query_rand()
{
	srand(time(NULL));

	clock_t start = clock(), diff;
	struct bt_cur cur;
	for(int i=0; i<n; i++) {
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

char *test_perf_insert_rand()
{
	srand(time(NULL));

	clock_t start = clock(), diff;
	for(int i=0; i<n; i++) {
		int item = rand();
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

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_btree_create);

	// Performance tests
	//mu_run_test(test_perf_insert_rand);
	//mu_run_test(test_perf_query_rand);

	// Basic tests
	mu_run_test(test_insert_dup);
	mu_run_test(test_query_basic);
	
	btree_debug_print(s, root, BTREE_DEBUG_FULL);
	
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
