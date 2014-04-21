#include "btree.h"
#include "bstack.h"

#include <src/util/dbg.h>

// TODO: support duplicates
// TODO: support variable length items
// TODO: doesn't work for small page sizes right now (median code broken)

#define BTREE_DEF_PAGE_SIZE 	64
#define BTREE_HEADER_SIZE		8

#define BTREE_ROOT 				1
#define BTREE_LEAF				2
#define BTREE_DUPS				4

// Some limits for error checking
#define BTREE_MIN_PAGE_SIZE		64
#define BTREE_MAX_PAGE_SIZE		65536

#define PG_HEADER(s,page)		(read_ui64(s, page, 0))

/*	Page header flags (1)
	hgfedcba
	a: is root?
	b: is leaf?
	c: has duplicates?
	d: variable-sized key?
	e: variable-sized data?
*/

#define PG_FLAGS(hdr)			((hdr >> 56) & 0xFFFF)
#define PG_SIZE(hdr)			((hdr >> 32) & 0xFFFF)
#define PG_NKEYS(hdr)			((hdr >> 16) & 0xFFFF)
#define PG_USED(hdr)			((hdr >> 0) & 0xFFFF)

#define PG_END(hdr)				(PG_USED(hdr))
#define PG_FREE(hdr)			(PG_SIZE(hdr) - PG_USED(hdr))
#define PG_START(hdr)			(BTREE_HEADER_SIZE)

#define IS_ROOT(hdr)			(PG_FLAGS(hdr) & BTREE_ROOT)
#define IS_LEAF(hdr)			(PG_FLAGS(hdr) & BTREE_LEAF)
#define IS_BRANCH(hdr)			(!(PG_FLAGS(hdr) & BTREE_LEAF))

#define KEY_GET(key)			(key & ~(1ull << 63))
#define KEY_HAS_OVERFLOW(key)	(key & (1ull << 63))
#define KEY_SET_OVERFLOW(key)	(key | (1ull << 63))

// INLINE HELPERS
static inline uint64_t read_ui64(struct store *s, page_p page, item_p i)
{
	return store_read_ui64(s, page + i);
}

static inline void write_ui64(struct store *s, page_p page, item_p i, uint64_t n)
{
	store_write_ui64(s, page + i, n);
}

static inline void copy(struct store *s, page_p dest, item_p di,
						page_p src, item_p si, item_p sz)
{
	store_copy(s, dest + di, src + si, sz);
}

// HEADER HELPERS
static inline void btree_hdr_write(struct store *s, page_p page,
										unsigned char flags,
										uint16_t sz_page,
										uint16_t n_keys,
										uint16_t used)
{
	unsigned char unused = 16;
	bt_header hdr = ((uint64_t) flags << 56) |
					((uint64_t) unused << 48) |
					((uint64_t) sz_page << 32) |
					((uint64_t) n_keys << 16) |
					((uint64_t) used << 0);
	write_ui64(s, page, 0, hdr);
}

// ITERATION HELPERS
static void btree_branch_walk(struct store *s, page_p page,
							  void (*iter)(struct store *, page_p, bt_key, page_p))
{
	bt_header hdr = PG_HEADER(s, page);
	item_p item = PG_START(hdr);

	page_p sub = read_ui64(s, page, item);
	iter(s, sub, 0, 0);
	item += sizeof(page_p);
	while (item < PG_END(hdr)) {
		bt_key key = read_ui64(s, page, item);
		page_p sub = read_ui64(s, page, item + sizeof(page_p));
		iter(s, 0, key, sub);
		item += sizeof(bt_key) + sizeof(page_p);
	}
}

static void btree_leaf_walk(struct store *s, page_p page,
							void (*iter)(struct store *, bt_key, bt_val))
{
	bt_header hdr = PG_HEADER(s, page);
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	item_p item = PG_START(hdr);

	while (item < PG_END(hdr)) {
		bt_key key = read_ui64(s, page, item);
		bt_val val = read_ui64(s, page, item + sizeof(key));
		iter(s, key, val);
		item += item_size;
	}
}

// DEBUG HELPERS
static inline void btree_debug_print_branch(struct store *s, page_p pre, bt_key key, page_p post)
{
	if (pre) {
		printf("\t(*%llu) ", pre);
	} else {
		printf("\t%llu (*%llu)", KEY_GET(key), post);
	}
}

static inline void btree_debug_print_overflow(struct store *s, bt_key key, bt_val val)
{
	if (KEY_HAS_OVERFLOW(key)) {
		bstack_debug_page(s, val);
	}
}

static inline void btree_debug_print_item(struct store *s, bt_key key, bt_val val)
{
	if (!KEY_HAS_OVERFLOW(key)) {
		printf("\t%llu:%llu", KEY_GET(key), val);
	} else {
		printf("\t%llu:(*%llu)", KEY_GET(key), val);
	}
}

static void btree_debug_page(struct store *s, page_p page)
{
	bt_header hdr = PG_HEADER(s, page);

	//uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	const char *prefix = IS_ROOT(hdr)? "root " : "";
	const char *type = IS_LEAF(hdr)? "leaf" : "branch";

	printf("----------------------------------------\n");
	printf("%s%s (*%llu) n: %llu \n"
				 "size: %llu, used: %llu, free: %llu [%d, %llu] \n",
				 prefix, type, page, PG_NKEYS(hdr),
				 PG_SIZE(hdr), PG_USED(hdr), PG_FREE(hdr),
				 PG_START(hdr), PG_END(hdr));

	if (IS_BRANCH(hdr)) {
		btree_branch_walk(s, page, &btree_debug_print_branch);
		printf("\n");
	} else {
		btree_leaf_walk(s, page, &btree_debug_print_item);
		printf("\n");
		btree_leaf_walk(s, page, &btree_debug_print_overflow);
	}
}

// PAGE SEARCH HELPERS
// TODO: make this a binary search
// Return index of child page that should contain key
static item_p btree_branch_find(struct store *s, page_p page, bt_key key)
{
	bt_header hdr = PG_HEADER(s, page);
	item_p item = PG_START(hdr);

	while (item < PG_END(hdr) - sizeof(page_p)) {
		if(key < read_ui64(s, page, item + sizeof(page_p))) break;
		item += sizeof(page_p) + sizeof(key);
	}
	return item;
}

// Look for key in page and return pointer to first item <= key
// TODO: make this a binary search
static item_p btree_leaf_find(struct store *s, page_p page, bt_key key)
{
	bt_header hdr = PG_HEADER(s, page);
	item_p item = PG_START(hdr);

	while (item < PG_END(hdr)) {
		if(key <= read_ui64(s, page, item)) return item;
		item += sizeof(key) + sizeof(bt_val);
	}
	return item;
}

// PAGE CREATION
// Create a new root branch
static inline page_p btree_root_create(struct store *s, bt_key mid,
									   page_p orig, page_p split)
{
	int item_size = sizeof(mid) + 2*sizeof(page_p);

	//TODO: move allocation somewhere else
	page_p root = store_alloc(s, BTREE_DEF_PAGE_SIZE);
	btree_hdr_write(s, root, BTREE_ROOT,
							 BTREE_DEF_PAGE_SIZE,
							 1,
							 BTREE_HEADER_SIZE + item_size);
	write_ui64(s, root, BTREE_HEADER_SIZE, orig);
	write_ui64(s, root, BTREE_HEADER_SIZE + sizeof(page_p), mid);
	write_ui64(s, root, BTREE_HEADER_SIZE + sizeof(page_p) + sizeof(mid), split);
	return root;
}

static inline void btree_branch_copy_up(struct store *s, page_p dest, page_p src,
										int n_keys, item_p si, item_p ei,
						  				bt_key key, page_p orig, page_p split,
						  				item_p i)
{
	bt_header hdr = PG_HEADER(s, src);
	int item_size = sizeof(key) + sizeof(page_p);

	debug("Branch copy up");
	debug("si: %hu, ei: %hu, i: %hu", si, ei, i);
	debug("key: %llu, orig: %llu, split: %llu", key, orig, split);

	if (i >= si && (si != PG_START(hdr) || i < ei)) {
		n_keys++;
		copy(s, dest, PG_START(hdr), src, si, i - si);
		write_ui64(s, dest, PG_START(hdr) + i - si, orig);
		write_ui64(s, dest, PG_START(hdr) + i - si + sizeof(key), key);
		write_ui64(s, dest, PG_START(hdr) + i - si + sizeof(key) + sizeof(page_p), split);
		i += sizeof(page_p);
		copy(s, dest, PG_START(hdr) + i - si + item_size, src, i, ei - i);
	} else {
		copy(s, dest, PG_START(hdr), src, si, ei - si);
	}

	btree_hdr_write(s, dest, PG_FLAGS(hdr) & ~BTREE_ROOT,
							 PG_SIZE(hdr),
							 n_keys,
							 n_keys*item_size + PG_START(hdr) + sizeof(page_p));
}

// Split a branch page
static inline bt_key btree_branch_split(struct store *s, page_p branch, item_p idx,
								 		bt_key key, page_p *orig, page_p *split)
{
	bt_header hdr = PG_HEADER(s, branch);
	uint32_t item_size = sizeof(key) + sizeof(page_p);

	// Allocate the new branches
	page_p left = store_alloc(s, PG_SIZE(hdr));
	page_p right = store_alloc(s, PG_SIZE(hdr));

	// Find middle value (median) of original leaf
	uint32_t n_keys_left = (PG_NKEYS(hdr) / 2);
	uint32_t n_keys_right = PG_NKEYS(hdr) - n_keys_left;
	item_p m = item_size * n_keys_left + PG_START(hdr) + sizeof(page_p);

	if (idx > m) {
		n_keys_left++;
		n_keys_right--;
		m += item_size;
	}

	bt_key mid = read_ui64(s, branch, m);

	debug("nleft: %u, nright: %u", n_keys_left, n_keys_right);

	// Copy over data to new leaves
	btree_branch_copy_up(s, left, branch, n_keys_left,
							PG_START(hdr), m,
							key, *orig, *split, idx);
	btree_branch_copy_up(s, right, branch, n_keys_right - 1,
							m + sizeof(mid), PG_END(hdr),
							key, *orig, *split, idx);

	*orig = left;
	*split = right;

	store_free(s, branch);
	return mid;
}

// Two cases:
//		1. 	There was room in the child node for the item
//				=> *orig contains copy of child page and *split = 0
//		2.	The child node had to split
//				=> *orig and *split contain the two split pages
//
// If this branch doesn't have room for split pages, returns 0
// btree_page_branch_split handles that case
static inline page_p btree_branch_add(struct store *s, page_p branch, item_p idx,
							   		  bt_key key, page_p *orig, page_p *split)
{
	bt_header hdr = PG_HEADER(s, branch);

	uint32_t item_size = sizeof(key) + sizeof(page_p);
	if (*split && PG_FREE(hdr) < item_size) return 0;

	page_p branch_copy = store_alloc(s, PG_SIZE(hdr));
	uint32_t n_keys = PG_NKEYS(hdr);

	int offset = 0;
	debug("idx = %hu", idx);

	copy(s, branch_copy, PG_START(hdr), branch, PG_START(hdr), idx - PG_START(hdr));
	write_ui64(s, branch_copy, idx, *orig);
	if (*split) {
		debug("Added new page to branch, mid = %llu", key);
		write_ui64(s, branch_copy, idx + sizeof(*orig), key);
		write_ui64(s, branch_copy, idx + sizeof(*orig) + sizeof(key), *split);
		offset = item_size;
		n_keys++;
	}
	copy(s, branch_copy, idx + sizeof(*orig) + offset,
			branch, idx + sizeof(*orig),
			PG_END(hdr) - idx - sizeof(*orig));

	btree_hdr_write(s, branch_copy, PG_FLAGS(hdr), PG_SIZE(hdr),
									n_keys, PG_USED(hdr) + offset);

	store_free(s, branch);
	return branch_copy;
}

static inline void btree_leaf_copy_in(struct store *s, page_p dest, page_p src,
									  int n_keys, item_p si, item_p ei,
									  bt_key key, bt_val val, item_p i)
{
	bt_header hdr = PG_HEADER(s, src);
	uint32_t item_size = sizeof(key) + sizeof(val);

	debug("Leaf copy in");
	debug("si: %hu, ei: %hu, i: %hu", si, ei, i);
	debug("key: %llu, val: %llu", key, val);

	if ((si == PG_START(hdr) || i > si) && i <= ei) {
		n_keys++;
		copy(s, dest, PG_START(hdr), src, si, i - si);
		write_ui64(s, dest, PG_START(hdr) + i - si, key);
		write_ui64(s, dest, PG_START(hdr) + i - si + sizeof(key), val);
		copy(s, dest, PG_START(hdr) + i - si + item_size, src, i, ei - i);
	} else {
		copy(s, dest, PG_START(hdr), src, si, ei - si);
	}

	btree_hdr_write(s, dest, PG_FLAGS(hdr) & ~BTREE_ROOT,
							 PG_SIZE(hdr),
							 n_keys,
							 n_keys*item_size + PG_START(hdr));
}

// Split a leaf page, return median to copy up tree
// TODO: remove knowledge of page structure
static inline bt_key btree_leaf_split(struct store *s, page_p leaf,
							   		  bt_key key, bt_val val,
									  page_p *orig, page_p *split)
{
	bt_header hdr = PG_HEADER(s, leaf);

	uint32_t item_size = sizeof(key) + sizeof(val);
	item_p i = btree_leaf_find(s, leaf, key);

	// Allocate the new leaves
	*orig = store_alloc(s, PG_SIZE(hdr));
	*split = store_alloc(s, PG_SIZE(hdr));

	// Find middle value (median) of original leaf
	// TODO: make find median a function
	uint32_t n_keys_left = (PG_NKEYS(hdr) / 2);
	uint32_t n_keys_right = PG_NKEYS(hdr) - n_keys_left;
	item_p m = item_size * n_keys_left + PG_START(hdr);

	if (i > m) {
		n_keys_left++;
		n_keys_right--;
		m += item_size;
	}

	bt_key mid = read_ui64(s, leaf, m);

	// Copy over data to new leaves
	btree_leaf_copy_in(s, *orig, leaf, n_keys_left, PG_START(hdr), m, KEY_GET(key), val, i);
	btree_leaf_copy_in(s, *split, leaf, n_keys_right, m, PG_END(hdr), KEY_GET(key), val, i);

	store_free(s, leaf);
	return mid;
}

// Add a duplicate key
// Duplicates are stored in a heap list of overflow pages
static inline page_p btree_leaf_add_dup(struct store *s, page_p page, item_p i,
										bt_key key, bt_val val)
{
	bt_header hdr = PG_HEADER(s, page);
	debug("Duplicate key: %llu", KEY_GET(key));

	// TODO: add option to disallow duplicates
	bt_val prev_val = read_ui64(s, page, i + sizeof(key));
	page_p overflow = prev_val;

	if (!KEY_HAS_OVERFLOW(key)) {
		// We need to create a new overflow page stack
		overflow = bstack_create(s);
		key = KEY_SET_OVERFLOW(key);
		debug("overflow page: %llu", overflow);

		// Add previous value to the overflow page list
		overflow = bstack_add(s, overflow, prev_val);
	}
	overflow = bstack_add(s, overflow, val);

	// Update the leaf page's pointer to the overflow page
	page_p page_copy = store_alloc(s, PG_SIZE(hdr));
	copy(s, page_copy, 0, page, 0, i);
	write_ui64(s, page_copy, i, key);
	write_ui64(s, page_copy, i + sizeof(key), overflow);
	i += sizeof(key) + sizeof(page_p);
	copy(s, page_copy, i, page, i, PG_END(hdr) - i);
	store_free(s, page);
	return page_copy;
}

// Try to add a key to the leaf page and return new page
// If page is full, return 0
static inline page_p btree_leaf_add(struct store *s, page_p page,
									bt_key key, bt_val val)
{
	bt_header hdr = PG_HEADER(s, page);
	uint32_t item_size = sizeof(key) + sizeof(val);

	item_p i = btree_leaf_find(s, page, key);

	// Call a different method for duplicate keys
	bt_key next_key = read_ui64(s, page, i);
	if (i < PG_END(hdr) && KEY_GET(next_key) == key)
		return btree_leaf_add_dup(s, page, i, next_key, val);

	// Exit if leaf page has no free space
	if (PG_FREE(hdr) < item_size) return 0;

	page_p page_copy = store_alloc(s, PG_SIZE(hdr));
	btree_hdr_write(s, page_copy, PG_FLAGS(hdr),
								  PG_SIZE(hdr),
								  PG_NKEYS(hdr)+1,
								  PG_USED(hdr) + item_size);

	// Copy over items in leaf with new item in correct spot
	copy(s, page_copy, PG_START(hdr), page, PG_START(hdr), i - PG_START(hdr));
	write_ui64(s, page_copy, i, key);
	write_ui64(s, page_copy, i + sizeof(key), val);
	copy(s, page_copy, i + item_size, page, i, PG_END(hdr) - i);

	store_free(s, page);
	return page_copy;
}

// Recursively add an item to the tree
// If a split occurs:
//		*orig <- new pointer to original page
//		*split <- new pointer to split page (if there is one)
//
// On return to original caller, the new root is contained in *orig
static bt_key btree_recur_add(struct store *s, page_p page,
							  bt_key key, bt_val val,
							  page_p *orig, page_p *split)
{
	bt_header hdr = PG_HEADER(s, page);

	if (IS_LEAF(hdr)) {
		*orig = btree_leaf_add(s, page, key, val);
		if(*orig) return key;

		debug("Need to split leaf page");
		return btree_leaf_split(s, page, key, val, orig, split);
	} else {
		item_p index = btree_branch_find(s, page, key);
		page_p sub = read_ui64(s, page, index);
		debug("Checking out page %llu", sub);

		bt_key mid = btree_recur_add(s, sub, key, val, orig, split);
		debug("*orig = %llu, *split = %llu", *orig, *split);

		page_p branch_copy = btree_branch_add(s, page, index, mid, orig, split);
		if(branch_copy != 0) {
			*orig = branch_copy;
			*split = 0;
			return mid;
		}

		debug("Need to split branch page");
		return btree_branch_split(s, page, index, mid, orig, split);
	}
}

// EXTERNAL API
page_p btree_create(struct store *s)
{
	page_p root = store_alloc(s, BTREE_DEF_PAGE_SIZE);
	btree_hdr_write(s, root, BTREE_LEAF | BTREE_ROOT,
							 BTREE_DEF_PAGE_SIZE,
							 0,
							 BTREE_HEADER_SIZE);
	return root;
}

// Free all pages of an entire sub-tree
// Call with root to destory the whole tree
void btree_destroy(struct store *s, page_p page)
{
	bt_header hdr = PG_HEADER(s, page);

	if (IS_LEAF(hdr)) {
		store_free(s, page);
	} else {
		// TODO: make iterating a function
		// Recursively destroy tree
		item_p item = PG_START(hdr);
		page_p sub = read_ui64(s, page, item);
		btree_destroy(s, sub);
		item += sizeof(page_p);
		while(item < PG_END(hdr)) {
			sub = read_ui64(s, page, item + 8);
			btree_destroy(s, sub);
			item += sizeof(bt_key) + sizeof(page_p);
		}
		store_free(s, page);
	}
}

// Find key in tree and return a cursor to its location
struct bt_cur btree_find(struct store *s, page_p root, bt_key key)
{
	check(s, "store is NULL");
	check(root, "root is not valid");

	bt_header hdr = PG_HEADER(s, root);

	if (IS_BRANCH(hdr)) {
		item_p index = btree_branch_find(s, root, key);
		page_p sub = read_ui64(s, root, index);
		return btree_find(s, sub, key);
	} else {
		item_p index = btree_leaf_find(s, root, key);
		return (struct bt_cur){.page = root, .index = index};
	}

error:
	return (struct bt_cur){.page = 0, .index = 0};
}

// Add key to tree and return pointer to new root node
page_p btree_add(struct store *s, page_p root, bt_key key, bt_val val)
{
	check(s, "store is NULL");
	check(root, "root is not valid");

	page_p orig = 0, split = 0;
	bt_key mid = btree_recur_add(s, root, key, val, &orig, &split);

	if (split != 0) {
		debug("Root needs to make like a tree and split :[");
		orig = btree_root_create(s, mid, orig, split);
	}
	return orig;

error:
	return root;
}

// Print out a btree for debugging
static void btree_debug_recur(struct store *s, page_p page,
							  int n, int level, int opts)
{
	bt_header hdr = PG_HEADER(s, page);
	btree_debug_page(s, page);
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	if (IS_BRANCH(hdr)) {
		// Recurse
		item_p item = PG_START(hdr);
		page_p sub = read_ui64(s, page, item);

		btree_debug_recur(s, sub, ++n, level+1, opts);
		item += sizeof(page_p);
		while(item < PG_END(hdr)) {
			sub = read_ui64(s, page, item + 8);
			btree_debug_recur(s, sub, ++n, level+1, opts);
			item += item_size;
		}
	}
}

void btree_debug_print(struct store *s, page_p root, int opts)
{
	printf("\n");
	printf("btree in %s\n", store_desc(s));

	btree_debug_recur(s, root, 0, 0, opts);
	printf("\n");
}
