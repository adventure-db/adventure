#include "btree.h"

#include <src/util/dbg.h>

// TODO: support duplicates
// TODO: support variable length items

#define BTREE_DEF_PAGE_SIZE 	128
#define BTREE_HEADER_SIZE		8

#define BTREE_ROOT 				0x01
#define BTREE_LEAF				0x02

// Some limits for error checking
#define BTREE_MIN_PAGE_SIZE		48
#define BTREE_MAX_PAGE_SIZE		65536

/*	Page header flags (1)
	hgfedcba
	a: is root?
	b: is leaf?
	c: has duplicates?
	d: variable-sized key?
	e: variable-sized data?
*/

// TODO: right now this is causing a read every time a macro is invoked
#define PG_HEADER(page)			(read_ui64(s, page, 0))

#define PG_FLAGS(page)			((PG_HEADER(page) >> 48) & 0xFFFF)
#define PG_SIZE(page)			((PG_HEADER(page) >> 32) & 0xFFFF)
#define PG_NKEYS(page)			((PG_HEADER(page) >> 16) & 0xFFFF)
#define PG_USED(page)			((PG_HEADER(page) >> 0) & 0xFFFF)

#define PG_END(page)			(PG_USED(page))
#define PG_FREE(page)			(PG_SIZE(page) - PG_USED(page))
#define PG_START(page)			(BTREE_HEADER_SIZE)

#define IS_ROOT(page)			(PG_FLAGS(page) & BTREE_ROOT)
#define IS_LEAF(page)			(PG_FLAGS(page) & BTREE_LEAF)
#define IS_BRANCH(page)			(!(PG_FLAGS(page) & BTREE_LEAF))

// INLINE HELPERS
/*
static inline uint16_t read_ui16(struct store *s, page_p page, item_p i)
{
	return store_read_ui16(s, page + i);
}
static inline uint32_t read_ui32(struct bt_page page, item_p i)
{
	return store_read_ui32(page.s, page.ptr + i);
}*/
static inline uint64_t read_ui64(struct store *s,
								 page_p page,
								 item_p i)
{
	return store_read_ui64(s, page + i);
}

static inline void write_ui16(struct store *s,
							  page_p page,
							  item_p i,
							  int n)
{
	store_write_ui16(s, page + i, n);
}/*
static inline void write_ui32(struct bt_page page, item_p i, uint32_t n)
{
	store_write_ui32(page.s, page.ptr + i, n);
}*/
static inline void write_ui64(struct store *s, page_p page, item_p i, int n)
{
	store_write_ui64(s, page + i, n);
}

static inline void copy(struct store *s, page_p dest, item_p di,
						page_p src, item_p si, item_p sz)
{
	store_copy(s, dest + di, src + si, sz);
}

// DEBUG HELPERS
static void btree_debug_page(struct store *s, page_p page)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	const char *prefix = IS_ROOT(page)? "root " : "";
	const char *type = IS_LEAF(page)? "leaf" : "branch";

	printf("----------------------------------------\n");
	printf("%s%s (%llu) n: %llu \n"
				 "size: %llu, used: %llu, free: %llu [%d, %llu] \n",
				 prefix, type, page, PG_NKEYS(page),
				 PG_SIZE(page), PG_USED(page), PG_FREE(page),
				 PG_START(page), PG_END(page));

	item_p item = PG_START(page);

	if (IS_BRANCH(page)) {
		page_p sub = read_ui64(s, page, item);
		printf("\t(%llu) ", sub);
		item += sizeof(bt_val);
		while(item < PG_END(page)) {
			bt_key key = read_ui64(s, page, item);
			page_p sub = read_ui64(s, page, item + 8);
			printf("\t%llu (%llu)", key, sub);
			item += item_size;
		}
		printf("\n");
	} else {
		while(item < PG_END(page)) {
			bt_key key = read_ui64(s, page, item);
			bt_val val = read_ui64(s, page, item + 8);
			printf("\t%llu:%llu", key, val);
			item += item_size;
		}
		printf("\n");
	}
}

// HEADER HELPERS
static inline void btree_page_hdr_write(struct store *s, page_p page,
										uint32_t flags,
										uint16_t sz_page,
										uint16_t n_keys,
										uint16_t used)
{
	write_ui16(s, page, 0, flags);
	write_ui16(s, page, 2, sz_page);
	write_ui16(s, page, 4, n_keys);
	write_ui16(s, page, 6, used);
}

// PAGE SEARCH HELPERS
// TODO: make this a binary search
// Return index of child page that should contain key
static item_p btree_branch_find(struct store *s, page_p page, bt_key key)
{
	item_p item = PG_START(page);
	while (item < PG_END(page) - sizeof(page_p)) {
		if(key < read_ui64(s, page, item + sizeof(page_p))) break;
		item += sizeof(page_p) + sizeof(key);
	}
	return item;
}

// Look for key in page and return pointer to first item <= key
// TODO: make this a binary search
static item_p btree_leaf_find(struct store *s, page_p page, bt_key key)
{
	item_p item = PG_START(page);
	while (item < PG_END(page)) {
		if(key < read_ui64(s, page, item)) return item;
		item += sizeof(key) + sizeof(bt_val);
	}
	return item;
}

// PAGE CREATION
// Create a new root branch
static page_p btree_root_create(struct store *s, bt_key mid,
								page_p orig, page_p split)
{
	int item_size = sizeof(mid) + 2*sizeof(page_p);

	//FIX: move allocation somewhere else
	page_p root = store_alloc(s, BTREE_DEF_PAGE_SIZE);
	btree_page_hdr_write(s, root, BTREE_ROOT, BTREE_DEF_PAGE_SIZE, 1,
							BTREE_HEADER_SIZE + item_size);

	write_ui64(s, root, PG_START(root), orig);
	write_ui64(s, root, PG_START(root) + sizeof(page_p), mid);
	write_ui64(s, root, PG_START(root) + sizeof(page_p) + sizeof(mid), split);
	return root;
}

static inline void btree_branch_copy_up(struct store *s, page_p dest, page_p src,
										int n_keys, item_p si, item_p ei,
						  				bt_key key, page_p orig, page_p split,
						  				item_p i)
{
	int item_size = sizeof(key) + sizeof(page_p);

	debug("Branch copy up");
	debug("si: %hu, ei: %hu, i: %hu", si, ei, i);
	debug("key: %llu, orig: %llu, split: %llu", key, orig, split);

	if (i >= si && (si != PG_START(src) || i < ei)) {
		n_keys++;
		i -= si;
		copy(s, dest, PG_START(src), src, si, i);
		write_ui64(s, dest, PG_START(src) + i, orig);
		write_ui64(s, dest, PG_START(src) + i + sizeof(key), key);
		write_ui64(s, dest, PG_START(src) + i + sizeof(key) + sizeof(page_p), split);
		i += sizeof(page_p);
		copy(s, dest, PG_START(src) + i + item_size, src, si + i, ei - si - i);
	} else {
		copy(s, dest, PG_START(src), src, si, ei - si);
	}

	btree_page_hdr_write(s, dest, PG_FLAGS(src) & ~BTREE_ROOT,
							PG_SIZE(src), n_keys,
							n_keys*item_size + PG_START(src) + sizeof(page_p));

	//btree_debug_page(s, src);
	//btree_debug_page(s, dest);
}

// Split a branch page
static bt_key btree_branch_split(struct store *s, page_p branch, item_p idx,
								 bt_key key, page_p *orig, page_p *split)
{
	
	uint32_t item_size = sizeof(key) + sizeof(page_p);

	// Allocate the new branches
	page_p left = store_alloc(s, PG_SIZE(branch));
	page_p right = store_alloc(s, PG_SIZE(branch));

	// Find middle value (median) of original leaf
	uint32_t n_keys_left = (PG_NKEYS(branch) / 2);
	uint32_t n_keys_right = PG_NKEYS(branch) - n_keys_left;
	item_p m = item_size * n_keys_left + PG_START(branch) + sizeof(page_p);

	if (idx > m && idx < PG_END(branch) - sizeof(page_p)) {
		n_keys_left++;
		n_keys_right--;
		m += item_size;
	}

	bt_key mid = read_ui64(s, branch, m);

	debug("nleft: %u, nright: %u", n_keys_left, n_keys_right);

	// Copy over data to new leaves
	btree_branch_copy_up(s, left, branch, n_keys_left,
							PG_START(branch), m,
							key, *orig, *split, idx);
	btree_branch_copy_up(s, right, branch, n_keys_right - 1,
							m + sizeof(mid), PG_END(branch),
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
static page_p btree_branch_add(struct store *s, page_p branch, item_p idx,
							   bt_key key, page_p *orig, page_p *split)
{
	uint32_t item_size = sizeof(key) + sizeof(page_p);
	if (*split && PG_FREE(branch) < item_size) return 0;

	page_p branch_copy = store_alloc(s, PG_SIZE(branch));
	uint32_t n_keys = PG_NKEYS(branch);

	int offset = 0;
	copy(s, branch_copy, 0, branch, 0, idx);
	write_ui64(s, branch_copy, idx, *orig);
	if (*split) {
		debug("Added new page to branch, mid = %llu", key);
		write_ui64(s, branch_copy, idx + sizeof(*orig), key);
		write_ui64(s, branch_copy, idx + sizeof(*orig) + sizeof(key), *split);
		offset += item_size;
		n_keys++;
	}
	copy(s, branch_copy, idx + sizeof(*orig) + offset,
			branch, idx + sizeof(*orig),
			PG_END(branch) - idx - sizeof(*orig));

	btree_page_hdr_write(s, branch_copy, PG_FLAGS(branch), PG_SIZE(branch),
							n_keys, PG_USED(branch) + offset);

	store_free(s, branch);
	return branch_copy;
}

static inline void btree_leaf_copy_in(struct store *s, page_p dest, page_p src,
									  int n_keys, item_p si, item_p ei,
									  bt_key key, bt_val val, item_p i)
{
	uint32_t item_size = sizeof(key) + sizeof(val);

	debug("Leaf copy in");
	debug("si: %hu, ei: %hu, i: %hu", si, ei, i);
	debug("key: %llu, val: %llu", key, val);

	if ((si == PG_START(src) || i > si) && i <= ei) {
		n_keys++;
		i -= si;
		copy(s, dest, PG_START(src), src, si, i);
		write_ui64(s, dest, PG_START(src) + i, key);
		write_ui64(s, dest, PG_START(src) + i + sizeof(key), val);
		copy(s, dest, PG_START(src) + i + item_size, src, si + i, ei - si - i);
	} else {
		copy(s, dest, PG_START(src), src, si, ei - si);
	}

	btree_page_hdr_write(s, dest, PG_FLAGS(src) & ~BTREE_ROOT, PG_SIZE(src),
							n_keys, n_keys*item_size + PG_START(src));

	//btree_debug_page(s, src);
	//btree_debug_page(s, dest);
}

// Split a leaf page, return median to copy up tree
// TODO: remove knowledge of page structure
static bt_key btree_leaf_split(struct store *s, page_p leaf,
							   bt_key key, bt_val val,
							   page_p *orig, page_p *split)
{
	uint32_t item_size = sizeof(key) + sizeof(val);
	item_p i = btree_leaf_find(s, leaf, key);

	// Allocate the new leaves
	*orig = store_alloc(s, PG_SIZE(leaf));
	*split = store_alloc(s, PG_SIZE(leaf));

	// Find middle value (median) of original leaf
	uint32_t n_keys_left = (PG_NKEYS(leaf) / 2);
	uint32_t n_keys_right = PG_NKEYS(leaf) - n_keys_left;
	item_p m = item_size * n_keys_left + PG_START(leaf);

	if (i > m && i < PG_END(leaf)) {
		n_keys_left++;
		n_keys_right--;
		m += item_size;
	}

	bt_key mid = read_ui64(s, leaf, m);

	// Copy over data to new leaves
	btree_leaf_copy_in(s, *orig, leaf, n_keys_left, PG_START(leaf), m, key, val, i);
	btree_leaf_copy_in(s, *split, leaf, n_keys_right, m, PG_END(leaf), key, val, i);

	store_free(s, leaf);
	return mid;
}

// Try to add a key to the leaf page and return new page
// If page is full, return 0
static page_p btree_leaf_add(struct store *s, page_p leaf,
							 bt_key key, bt_val val)
{
	uint32_t item_size = sizeof(key) + sizeof(val);
	if(PG_FREE(leaf) < item_size) return 0;

	// FIX: move to method
	page_p leaf_copy = store_alloc(s, PG_SIZE(leaf));
	btree_page_hdr_write(s, leaf_copy, PG_FLAGS(leaf), PG_SIZE(leaf),
							PG_NKEYS(leaf)+1, PG_USED(leaf) + item_size);

	item_p i = btree_leaf_find(s, leaf, key);
	debug("i: %hu, key: %llu", i, key);

	// Copy over items in leaf and wedge new item in the right spot
	copy(s, leaf_copy, PG_START(leaf), leaf, PG_START(leaf), i - PG_START(leaf));
	write_ui64(s, leaf_copy, i, key);
	write_ui64(s, leaf_copy, i + sizeof(key), val);
	copy(s, leaf_copy, i + item_size, leaf, i, PG_END(leaf) - i);

	//btree_debug_page(s, leaf);
	//btree_debug_page(s, leaf_copy);

	// FIX: move to method
	store_free(s, leaf);
	return leaf_copy;
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
	if (IS_LEAF(page)) {
		*orig = btree_leaf_add(s, page, key, val);
		if(*orig) return key;

		debug("Need to split leaf page");
		return btree_leaf_split(s, page, key, val, orig, split);
	} else {
		item_p index = btree_branch_find(s, page, key);
		page_p sub = read_ui64(s, page, index);
		debug("Checking out page %llu", sub);

		bt_key mid = btree_recur_add(s, sub, key, val, orig, split);
		debug("*split = %llu", *split);

		page_p branch_cpy = btree_branch_add(s, page, index, mid, orig, split);
		if(branch_cpy != 0) {
			*orig = branch_cpy;
			*split = 0;
			return mid;
		}
		return btree_branch_split(s, page, index, mid, orig, split);
	}
}

// EXTERNAL API
page_p btree_create(struct store *s)
{
	page_p root = store_alloc(s, BTREE_DEF_PAGE_SIZE);
	btree_page_hdr_write(s, root, BTREE_LEAF | BTREE_ROOT,
							BTREE_DEF_PAGE_SIZE, 0, BTREE_HEADER_SIZE);
	return root;
}

// TODO: implement
void btree_destroy(struct store *s, page_p root)
{
}

// Find key in tree and return a cursor to its location
// TODO: fix
struct bt_cur btree_find(struct store *s, page_p root, bt_key key)
{
	check(s, "store is NULL");
	check(root, "root is not valid");

	if (IS_BRANCH(root)) {
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
		store_free(s, root);
	}
	return orig;

error:
	return root;
}

// Print out a btree for debugging
static void btree_debug_page_print(struct store *s, page_p page,
								   int n, int level, int opts)
{
	btree_debug_page(s, page);
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	if (IS_BRANCH(page)) {
		// Recurse
		item_p item = PG_START(page);
		page_p sub = read_ui64(s, page, item);
		btree_debug_page_print(s, sub, ++n, level+1, opts);
		item += sizeof(page_p);
		while(item < PG_END(page)) {
			sub = read_ui64(s, page, item + 8);
			btree_debug_page_print(s, sub, ++n, level+1, opts);
			item += item_size;
		}
	}
}

void btree_debug_print(struct store *s, page_p root, int opts)
{
	printf("\n");
	printf("btree in %s\n", store_desc(s));

	btree_debug_page_print(s, root, 0, 0, opts);
	printf("\n");
}
