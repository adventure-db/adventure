#include "darray.h"
#include <src/util/dbg.h>

struct darray *darray_create(size_t max_size)
{
	struct darray *arr = calloc(1, sizeof(struct darray));
	check_mem(arr);

	arr->data = calloc(max_size, sizeof(void *));
	check_mem(arr->data);

	arr->size = 0;
	arr->max_size = max_size;
	return arr;

error:
	if(arr) free(arr);
	return NULL;
}

void darray_destroy(struct darray *arr)
{
	if(arr) {
		if(arr->data) free(arr->data);
		free(arr);
	}
}

int darray_reserve(struct darray *arr, size_t size)
{
	check(size > 0, "Cannot reserve space in array of size = 0");

	void *data = realloc(arr->data, size * sizeof(void *));
	check_mem(data);

	arr->max_size = size;
	arr->data = data;

	return 0;

error:
	return -1;
}

int darray_resize(struct darray *arr, size_t size, void *val)
{
	check( darray_reserve(arr, size) == 0, "Could not reserve space for resize" );

	// Set the expanded data to the default value
	for(int i = arr->size; i < size; i++) {
		arr->data[i] = val;
	}

	arr->size = size;

	return 0;

error:
	return -1;
}

int darray_expand(struct darray *arr)
{
	return darray_reserve(arr, arr->max_size * 2);
}

int darray_contract(struct darray *arr)
{
	if(arr->max_size < 2) return 0;
	return darray_reserve(arr, arr->max_size / 2);
}

void darray_set(struct darray *arr, size_t i, void *elem)
{
	check(i < arr->max_size, "Tried to index beyond allocated array size");
	if(i > arr->size) arr->size = i;
	arr->data[i] = elem;

error:
	return;
}

void *darray_get(struct darray *arr, size_t i)
{
	check(i < arr->size, "Tried to index beyond array dimensions");
	return arr->data[i];

error:
	return NULL;
}

int darray_push(struct darray *arr, void *elem)
{
	if(arr->size >= arr->max_size) {
		check(darray_expand(arr) == 0, "Could not expand array on push");
	}

	arr->data[arr->size] = elem;
	arr->size++;
	return 0;

error:
	return -1;
}

void *darray_pop(struct darray *arr)
{
	check(arr->size > 0, "Cannot pop from an empty array");
	arr->size--;
	return arr->data[arr->size];

error:
	return NULL;
}
