#ifndef ADV_HASH_H
#define ADV_HASH_H

#include <stdint.h>

typedef struct hashtbl
{
	void **data;
	uint32_t n;
	uint32_t size;
};

typedef struct hashblock
{
	void *key;
	void *val;
	uint32_t hash;
};

unsigned long simplehash(void *elem)
{

}

#endif