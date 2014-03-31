#ifndef ADV_DARRAY_H
#define ADV_DARRAY_H

#include <stdlib.h>

struct darray
{
	void **data;

	size_t elem_size;
	size_t init_max;
};

struct darray *darray_create(size_t elem_size, size_t init_max);
void darray_destroy(struct darray *arr);

#endif
