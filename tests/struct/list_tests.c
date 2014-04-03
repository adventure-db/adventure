#include <tests/minunit.h>
#include <src/struct/list.h>

static struct list *list = NULL;
char *test1 = "test1 data";
char *test2 = "test2 data";
char *test3 = "test3 data";

char *test_create()
{
	list = list_create();
	mu_assert(list != NULL, "Failed to create list");
	return NULL;
}

char *test_destroy()
{
	list_clear(list);
	list_destroy(list);
	return NULL;
}

char *test_push_pop()
{
	list_push(list, test1);
	mu_assert(list_last(list) == test1, "Wrong last value");

	list_push(list, test2);
	mu_assert(list_last(list) == test2, "Wrong last value");

	list_push(list, test3);
	mu_assert(list_last(list) == test3, "Wrong last value");

	char *val = list_pop(list);
	mu_assert(val == test3, "Wrong value on pop");

	val = list_pop(list);
	mu_assert(val == test2, "Wrong value on pop");

	val = list_pop(list);
	mu_assert(val == test1, "Wrong value on pop");
	mu_assert(list_count(list) == 0, "Wrong count");

	return NULL;
}

char *test_shift_unshift()
{
	list_unshift(list, test1);
	mu_assert(list_last(list) == test1, "Wrong last value");

	list_unshift(list, test2);
	mu_assert(list_first(list) == test2, "Wrong first value");
	mu_assert(list_last(list) == test1, "Wrong last value");

	list_unshift(list, test3);
	mu_assert(list_first(list) == test3, "Wrong first value");
	mu_assert(list_last(list) == test1, "Wrong last value");

	char *val = list_shift(list);
	mu_assert(val == test3, "Wrong value on unshift");

	val = list_shift(list);
	mu_assert(val == test2, "Wrong value on unshift");

	val = list_shift(list);
	mu_assert(val == test1, "Wrong value on unshift");
	mu_assert(list_count(list) == 0, "Wrong count");

	return NULL;
}

char *test_remove()
{
	char *val = list_remove(list, list->first->next);
	mu_assert(val == test2, "Wrong removed element");

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_push_pop);
	mu_run_test(test_shift_unshift);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
