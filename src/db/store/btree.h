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
			type: BTREE_FLAG_LEAF | BTREE_FLAG_BRANCH
		n_keys (2): number of keys held
		used (2): used space

	BRANCH PAGE (BTREE_PAGE_SIZE)
	ptr 0 (8)
	key 0 (8)
	...
	key i (8)
	ptr i+1 (8)

	LEAF PAGE (BTREE_PAGE_SIZE)
	key i (8)
	val i (8)
*/

typedef uint64_t bt_pg;
typedef uint64_t bt_key;
typedef uint64_t bt_val;

store_p btree_alloc(struct store *s);
void btree_free(struct store *s, store_p root);

store_p btree_add(struct store *s, store_p root, bt_key key, bt_val val);
store_p btree_get(struct store *s, store_p root, bt_key key);
store_p btree_remove(struct store *s, store_p root, bt_key key);

void btree_debug_print(struct store *s, store_p root, int opts);
void btree_debug_audit(struct store *s, store_p root);

#endif
