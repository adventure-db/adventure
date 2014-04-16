#include <tests/minunit.h>
#include <src/db/store/store.h>
#include <string.h>
#include <time.h>

static struct store *s = NULL;
static store_p ptr = 0;
static char *data1 = "Hello";

char *test_open()
{
	s = store_open("testfile_nodes", STORE_O_CREAT);
	mu_assert(s != NULL, "File should not be NULL");

	return NULL;
}

char *test_extend()
{
	size_t size = s->file->size;
	store_extend(s);
	mu_assert(s->file->size = size * 2, "File size not doubled");

	return NULL;
}

char *test_alloc()
{
	store_p p = store_alloc(s, 1024*1024*2);
	store_free(s, p);

	return NULL;
}

char *test_read_write()
{
	ptr = store_alloc(s, 5);

	store_write(s, ptr, data1, 5);
	char *val = store_read(s, ptr, 5);
	mu_assert(strncmp(val, data1, 5) == 0, "Should read 'Hello' from file");

	return NULL;
}

char *test_copy()
{
	store_p p = store_alloc(s, 5);
	store_copy(s, p, ptr, 5);

	char *val1 = store_read(s, ptr, 5);
	char *val2 = store_read(s, p, 5);

	mu_assert(strncmp(val1, val2, 5) == 0, "Copied area should contain 'Hello'");

	return NULL;
}

char *test_large_copy()
{
	store_p p_old = store_alloc(s, 1024*1024*100);
	store_p p_new = store_alloc(s, 1024*1024*100);

	clock_t start = clock(), diff;
	store_copy(s, p_new, p_old, 1024*1024*100);
	diff = clock() - start;

	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("Time for 100MB copy in ms: %d\n", msec);

	return NULL;
}

char *test_close()
{
	store_close(s);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_open);
	mu_run_test(test_read_write);
	mu_run_test(test_alloc);
	mu_run_test(test_copy);
	mu_run_test(test_extend);
	mu_run_test(test_large_copy);
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
