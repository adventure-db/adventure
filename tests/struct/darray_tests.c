#include <tests/minunit.h>
#include <src/struct/darray.h>

static struct darray *arr = NULL;
char *test1 = "test1 data";
char *test2 = "test2 data";
char *test3 = "test3 data";

char *test_create()
{
	arr = darray_create( 2 );
	mu_assert(arr != NULL, "Array should not be null");
	mu_assert(arr->max_size, "Max size of array should be 2");
	mu_assert(arr->size == 0, "Size of array should be 0");
	mu_assert(arr->data, "Array data should be allocated");

	return NULL;
}

char *test_destroy()
{
	darray_destroy(arr);

	return NULL;
}

char *test_push_pop()
{
	darray_push(arr, test1);
	mu_assert(darray_get(arr, 0) == test1, "Wrong value on push");
	mu_assert(arr->size == 1, "Array size should be 1");

	darray_push(arr, test2);
	mu_assert(darray_get(arr, 1) == test2, "Wrong value on push");
	mu_assert(arr->size == 2, "Array size should be 2");

	void *elem = darray_pop(arr);
	mu_assert(elem == test2, "Wrong value on pop");
	mu_assert(arr->size == 1, "Array size should be 1");

	elem = darray_pop(arr);
	mu_assert(elem == test1, "Wrong value on pop");
	mu_assert(arr->size == 0, "Array size should be 0");

	elem = darray_pop(arr);
	mu_assert(elem == NULL, "Popped empty array - element should be NULL");
	mu_assert(arr->size == 0, "Array size should be 0");

	return NULL;
}

char *test_auto_expand()
{
	int n = 10000;
	for(int i=0; i<n; i++) {
		darray_push(arr, test1);
	}

	mu_assert(arr->size == n, "Array size should be == n");
	mu_assert(arr->max_size >= n && arr->max_size <= 2*n, "Array max size should be >= n and <= 2*n");

	for(int i=0; i<n; i++) {
		darray_pop(arr);
	}

	mu_assert(arr->size == 0, "Array size should be 0");

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_push_pop);
	mu_run_test(test_auto_expand);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
