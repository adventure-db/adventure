#ifndef ADV_DB_STORE_sa
#define ADV_DB_STORE_sa

#include <stdint.h>
#include "dbf.h"

/*
	Segmented array
*/

struct sa
{
	struct dbf *f;
	size_t buckets;
};

struct sa *sa_create(void *mem, size_t size);
void sa_destroy(struct sa *sa);

// Update operations within a bucket
int sa_add(struct sa *sa, dbf_p p, void *data, size_t size);
int sa_remove(struct sa *sa, dbf_p p);
dbf_p sa_get(struct sa *sa, dbf_p p);

#endif
