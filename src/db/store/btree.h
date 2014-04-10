#ifndef ADV_DB_STORE_BTREE_H
#define ADV_DB_STORE_BTREE_H

#include <stdint.h>
#include "store.h"

#define BTREE_DEBUG_FULL	0x01

// TODO: separate out page representation from general algorithm

/*	Copy-on-write btree

	Unique points:
	-	All inserts, updates and deletes result in a copy of the tree path
		and a new root node.

		Allocation of pages is the responsibility of the backing store.
		This gives the backing store flexibility in determining whether to
		do append-only writes or reclaim free space (or even be adaptive and
		choose its strategy based on the write load)

	-	Very lightweight with no stdlib memory allocation.
		Designed for applications that could potentially have millions of trees
		in a single file.

	-	Does not handle meta pages. It is the responsibility of the user of the
		btree to maintain a reference to the root.

		Failure to maintain a reference to root would result in a leak.

	PAGE LAYOUT
	--------------------------------
	HEADER (8)
		flags (2): flags =
					BTREE_FLAG_LEAF | BTREE_FLAG_BRANCH | BTREE_FLAG_META
		page sz (2): size of this page
		n_keys (2): number of keys held
		end (2): end of the page

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

typedef uint64_t bt_key;
typedef uint64_t bt_val;
typedef uint16_t item_p;

// TODO: make this smaller
struct bt_page
{
	struct store *s;
	store_p ptr;
	uint16_t flags;
	uint16_t sz_page;
	uint16_t sz_free;
	uint16_t n_keys;
	uint16_t start;
	uint16_t end;
};

struct bt_cur
{
	struct bt_page page;
	item_p index;
};

struct bt_page btree_create(struct store *s);
struct bt_page btree_page(struct store *s, store_p root);
void btree_destroy(struct bt_page root);

struct bt_page btree_add(struct bt_page root, bt_key key, bt_val val);
struct bt_cur btree_find(struct bt_page root, bt_key key);
struct bt_page btree_remove(struct bt_page root, bt_key key);

void btree_debug_print(struct bt_page root, int opts);

#endif
