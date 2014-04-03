#ifndef ADV_DARRAY_H
#define ADV_DARRAY_H

#include <stdlib.h>

struct darray
{
	void **data;
	size_t size;
	size_t max_size;
};

struct darray *darray_create(size_t init_max);
void darray_destroy(struct darray *arr);

int darray_reserve(struct darray *arr, size_t size);
int darray_resize(struct darray *arr, size_t size, void *val);
int darray_expand(struct darray *arr);
int darray_contract(struct darray *arr);

int darray_push(struct darray *arr, void *elem);
void *darray_pop(struct darray *arr);

void darray_set(struct darray *arr, size_t i, void *elem);
void *darray_get(struct darray *arr, size_t i);

#endif
