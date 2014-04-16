#ifndef ADV_DB_STORE_BTREE_H
#define ADV_DB_STORE_BTREE_H

#include <stdint.h>
#include "store.h"

#define BTREE_DEBUG_FULL			0x01
#define BTREE_DEBUG_OVERVIEW		0x02

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

	PAGE LAYOUT
	--------------------------------
	HEADER (8)
		flags (2): flags =
					BTREE_LEAF | BTREE_BRANCH
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
typedef uint64_t bt_header;
typedef uint16_t item_p;
typedef store_p page_p;

// Cursor to a specific item
struct bt_cur
{
	page_p page;
	item_p index;
};

page_p btree_create(struct store *s);
void btree_destroy(struct store *s, page_p root);

page_p btree_add(struct store *s, page_p root, bt_key key, bt_val val);
struct bt_cur btree_find(struct store *s, page_p root, bt_key key);
page_p btree_remove(struct store *s, page_p root, bt_key key);

void btree_debug_print(struct store *s, page_p root, int opts);

#endif
