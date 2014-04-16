#include <tests/minunit.h>
#include <src/db/adv.h>
#include <string.h>

static struct adv_db *db = NULL;

char *test_open()
{
	db = adv_open("test", ADV_CREATE);

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

	mu_run_test(test_open);
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
