#include <tests/minunit.h>
#include <src/db/store/dbf.h>
#include <string.h>

static struct dbf *f = NULL;
static dbf_p ptr = 0;
static char *data1 = "Hello";

char *test_open()
{
	f = dbf_open("testfile_nodes", DBF_O_CREAT);
	mu_assert(f != NULL, "File should not be NULL");

	return NULL;
}

char *test_extend()
{
	size_t size = f->file->size;
	dbf_extend(f);
	mu_assert(f->file->size = size * 2, "File size not doubled");

	return NULL;
}

char *test_alloc()
{
	dbf_p p = dbf_alloc(f, 1024*1024*2);
	dbf_free(f, p);

	return NULL;
}

char *test_read_write()
{
	ptr = dbf_alloc(f, 5);

	dbf_write(f, ptr, data1, 5);
	char *val = dbf_read(f, ptr, 5);
	mu_assert(strncmp(val, data1, 5) == 0, "Should read 'Hello' from file");

	return NULL;
}

char *test_copy()
{
	dbf_p p = dbf_alloc(f, 5);
	dbf_copy(f, p, ptr, 5);

	char *val1 = dbf_read(f, ptr, 5);
	char *val2 = dbf_read(f, p, 5);

	mu_assert(strncmp(val1, val2, 5) == 0, "Copied area should contain 'Hello'");

	return NULL;
}

char *test_close()
{
	dbf_close(f);

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_open);
	mu_run_test(test_read_write);
	mu_run_test(test_alloc);
	mu_run_test(test_copy);
	mu_run_test(test_extend);
	mu_run_test(test_close);

	return NULL;
}

RUN_TESTS(all_tests);
