#include <tests/minunit.h>
#include <src/db/db.h>
#include <string.h>

static struct adv_db *db = NULL;

char *test_open()
{
	db = adv_open("test", ADV_CREATE);

	return NULL;
}

char *test_stress()
{
	int n = 1000000;
	int m = 100000;

	for(int i=0; i<n; i++) {
		adv_node_add(db);
	}
	printf("Added %d nodes\n", n);

	for(int j=0; j<m; j++) {
		adv_edge_add(db, 1, j+2, 0);
	}
	printf("Added %d edges from node 1", m);

	return NULL;
}

char *test_close()
{
	adv_close(db);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_stress);
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
