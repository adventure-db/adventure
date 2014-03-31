#ifndef ADV_HASH_H
#define ADV_HASH_H

struct hashtbl
{
	void **data;
	uint32_t n;
	uint32_t size;
};

struct hashblock
{
	void *key;
	void *val;
	uint32_t hash;
};

unsigned long simplehash(void *elem)
{

}

#endif