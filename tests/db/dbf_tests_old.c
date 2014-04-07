#include <tests/minunit.h>
#include <src/db/store/dbf.h>
#include <string.h>

static struct dbf *dbf = NULL;

char *test_create()
{
	dbf = dbf_open("testfile_nodes", DBF_O_CREAT);
	mu_assert(dbf != NULL, "File should not be NULL");
	mu_assert(dbf->init_size == 20, "Initial power of 2 size should be 20");
	mu_assert(dbf->n_files == 1, "Number of volumes should be 1");
	mu_assert(strcmp(dbf->name, "testfile_nodes") == 0, "File name should be testfile_nodes");

	return NULL;
}

char *test_open()
{
	dbf = dbf_open("testfile_nodes", DBF_O_CREAT);
	mu_assert(dbf != NULL, "File should not be NULL");
	mu_assert(dbf->init_size == 20, "Initial power of 2 size should be 20");
	mu_assert(dbf->n_files == 5, "Number of volumes should be 1");
	mu_assert(strcmp(dbf->name, "testfile_nodes") == 0, "File name should be testfile_nodes");

	return NULL;
}

char *test_add_vols()
{
	for(int i=0; i<4; i++) {
		dbf_add_vol(dbf);
		mu_assert(dbf->file[i+1] != NULL, "Volume should not be null");
	}
	mu_assert(dbf->n_files == 5, "Number of volumes should be 5");

	return NULL;
}

char *test_add_more_vols()
{
	for(int i=0; i<4; i++) {
		dbf_add_vol(dbf);
		mu_assert(dbf->file[i+1] != NULL, "Volume should not be null");
	}
	mu_assert(dbf->n_files == 9, "Number of volumes should be 5");

	return NULL;
}

char *test_close()
{
	dbf_close(dbf);

	return NULL;
}

char *test_delete()
{
	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	// First create a bunch of volumes
	mu_run_test(test_create);
	mu_run_test(test_add_vols);
	mu_run_test(test_close);

	// Re-open those volumes and create some more
	mu_run_test(test_open);
	mu_run_test(test_add_more_vols);
	mu_run_test(test_close);

	// Delete those volumes
	mu_run_test(test_delete);

	// Create a file and insert data

	return NULL;
}

RUN_TESTS(all_tests);
