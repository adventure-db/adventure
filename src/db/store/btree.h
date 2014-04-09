#ifndef ADV_DB_STORE_BTREE_H
#define ADV_DB_STORE_BTREE_H

#include <stdint.h>
#include "store.h"

#define BTREE_DEBUG_FULL	0x01

// TODO: separate out page representation from general algorithm

/*	Append-only copy-on-write btree

	PAGE LAYOUT
	--------------------------------
	HEADER (8)
		flags (4): flags =
			type (1): BTREE_FLAG_LEAF | BTREE_FLAG_BRANCH | BTREE_FLAG_META
		n_keys (2): number of keys held
		used (2): used space

	META PAGE (BTREE_PAGE_SIZE)
	// TODO

	BRANCH PAGE (BTREE_PAGE_SIZE)
	ptr 0 (8)
	key 0 (8)
	...
	key n (8)
	ptr n+1 (8)

	LEAF PAGE (BTREE_PAGE_SIZE)
	key 0 (8)
	val 0 (8)
	...
	key n (8)
	val n (8)
*/

struct bt_meta
{
	uint32_t page_sz;
	uint32_t hdr_sz;
	uint32_t depth;
};

struct bt_page
{
	struct store *s;
	store_p ptr;
	store_p meta;
	//struct bt_hdr header;
};

typedef uint64_t bt_key;
typedef uint64_t bt_val;

struct bt_page btree_page(struct store *s, store_p root);

struct bt_page btree_alloc(struct store *s);
void btree_free(struct bt_page root);

store_p btree_add(struct store *s, store_p root, bt_key key, bt_val val);
store_p btree_find(struct store *s, store_p root, bt_key key);
store_p btree_remove(struct store *s, store_p root, bt_key key);

void btree_debug_print(struct store *s, store_p root, int opts);
int btree_audit(struct store *s, store_p root);

#endif
