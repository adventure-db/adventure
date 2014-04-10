#include "btree.h"

#include <src/util/dbg.h>

#define BTREE_DEF_PAGE_SIZE 	64
#define BTREE_HEADER_SIZE		8

#define BTREE_ROOT 				0x01
#define BTREE_BRANCH			0x02
#define BTREE_LEAF				0x04

#define PG_FLAGS(page)			(page.flags)
#define PG_SIZE(page)			(page.sz_page)
#define PG_FREE(page)			(page.sz_free)
#define PG_USED(page)			(page.end)
#define PG_START(page)			(page.start)
#define PG_END(page)			(page.end)
#define PG_NKEYS(page)			(page.n_keys)

#define IS_ROOT(page)			(page.flags & BTREE_ROOT)
#define IS_LEAF(page)			(page.flags & BTREE_LEAF)
#define IS_BRANCH(page)			(page.flags & BTREE_BRANCH)

// INLINE HELPERS
static inline uint16_t read_ui16(struct bt_page page, item_p i)
{
	return store_read_ui16(page.s, page.ptr + i);
}/*
static inline uint32_t read_ui32(struct bt_page page, item_p i)
{
	return store_read_ui32(page.s, page.ptr + i);
}*/
static inline uint64_t read_ui64(struct bt_page page, item_p i)
{
	return store_read_ui64(page.s, page.ptr + i);
}

static inline void write_ui16(struct bt_page page, item_p i, uint16_t n)
{
	store_write_ui16(page.s, page.ptr + i, n);
}/*
static inline void write_ui32(struct bt_page page, item_p i, uint32_t n)
{
	store_write_ui32(page.s, page.ptr + i, n);
}*/
static inline void write_ui64(struct bt_page page, item_p i, uint64_t n)
{
	store_write_ui64(page.s, page.ptr + i, n);
}

static inline void copy(struct bt_page dest, item_p di, struct bt_page src, item_p si, item_p sz)
{
	store_copy(dest.s, dest.ptr + di, src.ptr + si, sz);
}

// DEBUG HELPERS
static inline void btree_debug_page(struct bt_page page)
{
	const char *prefix = IS_ROOT(page)? "root " : "";
	const char *type = IS_LEAF(page)? "leaf" : "branch";

	debug("%s%s page at %llu: size: %u, used: %u, free: %u", prefix, type, page.ptr, PG_SIZE(page), PG_USED(page), PG_FREE(page));
	debug("\tflags: %u", PG_FLAGS(page));
	debug("\tn_keys: %hu", PG_NKEYS(page));
	debug("\tstart: %hu", PG_START(page));
	debug("\tend: %hu", PG_END(page));
}

// HEADER HELPERS
static inline void btree_page_hdr_read(struct bt_page *page)
{
	page->flags = read_ui16(*page, 0);
	page->sz_page = read_ui16(*page, 2);
	page->n_keys = read_ui16(*page, 4);
	page->end = read_ui16(*page, 6);
	page->sz_free = page->sz_page - page->end;
	page->start = BTREE_HEADER_SIZE;
}

static inline void
btree_page_hdr_write(struct bt_page *page, uint32_t flags, uint16_t sz_page, uint16_t n_keys, uint16_t used)
{
	write_ui16(*page, 0, flags);
	write_ui16(*page, 2, sz_page);
	write_ui16(*page, 4, n_keys);
	write_ui16(*page, 6, used);
	btree_page_hdr_read(page);
}

// PAGE SEARCH HELPERS
// TODO: make this a binary search
static struct bt_page btree_page_branch_find(struct bt_page page, bt_key key)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	store_p item = PG_START(page);
	bt_val val = read_ui64(page, item);
	item += sizeof(bt_val);
	while(item < PG_END(page)) {
		if(key <= read_ui64(page, item)) break;
		val = read_ui64(page, item + 8);
		item += item_size;
	}
	return btree_page(page.s, val);
}

// Look for key in page and return pointer to first item <= key
// TODO: make this a binary search
static item_p btree_page_leaf_find(struct bt_page page, bt_key key)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	item_p item = PG_START(page);
	while(item < PG_END(page)) {
		if(key <= read_ui64(page, item)) break;
		item += item_size;
	}
	return item;
}

// PAGE CREATION
// Create a new root branch
static struct bt_page btree_root_create(struct store *s, bt_key mid, struct bt_page p1, struct bt_page p2)
{
	uint32_t item_size = sizeof(bt_key) + 2*sizeof(bt_val);

	//FIX: move allocation somewhere else
	struct bt_page root;
	root.s = s;
	root.ptr = store_alloc(root.s, BTREE_DEF_PAGE_SIZE);
	btree_page_hdr_write(&root, BTREE_ROOT | BTREE_BRANCH, BTREE_DEF_PAGE_SIZE, 1, BTREE_HEADER_SIZE + item_size);

	write_ui64(root, PG_START(root), p1.ptr);
	write_ui64(root, PG_START(root) + sizeof(bt_val), mid);
	write_ui64(root, PG_START(root) + sizeof(bt_val) + sizeof(bt_key), p2.ptr);
	return root;
}

// Split a branch page
static bt_key btree_page_branch_split(struct bt_page page, bt_key key, struct bt_page *p1, struct bt_page *p2)
{
	return 0;
}

static struct bt_page btree_page_branch_add(struct bt_page branch, struct bt_page sub, struct bt_page *orig, struct bt_page *split)
{

	return branch;
}

static inline void
btree_page_copy_in(struct bt_page dest, struct bt_page src, uint32_t n_keys, item_p s, item_p e, bt_key key, bt_val val, item_p i)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	if((s == 0 || i > s) && i <= e) {
		n_keys++;
		i -= s;
		copy(dest, src.start, src, src.start + s, i);
		write_ui64(dest, src.start + i, key);
		write_ui64(dest, src.start + i + sizeof(bt_key), val);
		copy(dest, src.start + i + item_size, src, src.start + s + i, e - i);
	} else {
		copy(dest, src.start, src, src.start + s, e - s);
	}

	btree_page_hdr_write(&dest, src.flags & ~BTREE_ROOT, src.sz_page, n_keys, n_keys*item_size);
}

// Split a leaf page, return median to copy up tree
// TODO: remove knowledge of page structure
static bt_key btree_page_leaf_split(struct bt_page leaf, bt_key key, bt_val val, struct bt_page *orig, struct bt_page *split)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	item_p i = btree_page_leaf_find(leaf, key) - leaf.start;

	// Allocate the new leaves
	orig->ptr = store_alloc(leaf.s, BTREE_DEF_PAGE_SIZE);
	split->ptr = store_alloc(leaf.s, BTREE_DEF_PAGE_SIZE);

	// Find middle value (median) of original leaf
	uint32_t n_keys_orig = (leaf.n_keys / 2);
	uint32_t n_keys_split = leaf.n_keys - n_keys_orig;
	item_p m = item_size * n_keys_orig;
	debug("i: %hu", i);
	debug("m: %hu", m);
	bt_key mid = read_ui64(leaf, leaf.start + m);

	// Copy over data to new leaves
	btree_page_copy_in(*orig, leaf, n_keys_orig, 0, m, key, val, i);
	btree_page_copy_in(*split, leaf, n_keys_split, m, leaf.end - m, key, val, i);

	store_free(leaf.s, leaf.ptr);
	return mid;
}

// Try to add a key to the leaf page and return new page
// If page is full, return 0
static struct bt_page btree_page_leaf_add(struct bt_page leaf, bt_key key, bt_val val)
{
	uint32_t item_size = sizeof(key) + sizeof(val);
	if(leaf.sz_free < item_size) return leaf;	// FIX: not right

	// FIX: move to method
	struct bt_page leaf_copy;
	leaf_copy.s = leaf.s;
	leaf_copy.ptr = store_alloc(leaf.s, leaf.sz_page);
	btree_page_hdr_write(&leaf_copy, leaf.flags, leaf.sz_page, leaf.n_keys+1, leaf.end + item_size);

	item_p i = btree_page_leaf_find(leaf, key);

	// Copy over items in leaf and wedge in new item in the right spot
	copy(leaf_copy, 0, leaf, 0, i);
	write_ui64(leaf_copy, i, key);
	write_ui64(leaf_copy, i + sizeof(bt_key), val);
	copy(leaf_copy, i + item_size, leaf, i, leaf.end - i);

	// FIX: move to method
	store_free(leaf.s, leaf.ptr);
	return leaf_copy;
}

// Recursively add an item to the tree
// If a split occurs:
//		orig <- new pointer to original page
//		split <- new pointer to split page (if there is one)
static bt_key btree_recur_add(struct bt_page page, bt_key key, bt_val val, struct bt_page *orig, struct bt_page *split)
{
	btree_debug_page(page);

	if(IS_LEAF(page)) {
		*orig = btree_page_leaf_add(page, key, val);
		if(orig->ptr != page.ptr) return key;

		debug("Need to split leaf page");
		return btree_page_leaf_split(page, key, val, orig, split);
	} else {
		struct bt_page sub = btree_page_branch_find(page, key);
		debug("Found branch %llu", sub.ptr);

		bt_key mid = btree_recur_add(sub, key, val, orig, split);
		debug("mid: %llu", mid);

		struct bt_page page_copy = btree_page_branch_add(page, sub, orig, split);
		*orig = page;	//FIX
		if(page_copy.ptr != page.ptr) return mid;
		return btree_page_branch_split(page, mid, orig, split);
	}
}

// EXTERNAL API
// TODO: inline this?
struct bt_page btree_page(struct store *s, store_p p)
{
	struct bt_page page;
	page.s = s;
	page.ptr = p;
	btree_page_hdr_read(&page);
	return page;
}

struct bt_page btree_create(struct store *s)
{
	struct bt_page root;
	root.s = s;
	root.ptr = store_alloc(s, BTREE_DEF_PAGE_SIZE);
	btree_page_hdr_write(&root, BTREE_LEAF | BTREE_ROOT, BTREE_DEF_PAGE_SIZE, 0, BTREE_HEADER_SIZE);
	return root;
}

// TODO: implement
void btree_destroy(struct bt_page root)
{
}

// Find key in tree and return a cursor to its location
struct bt_cur btree_find(struct bt_page root, bt_key key)
{
	//check(root.s, "store is NULL");
	//check(root.ptr, "root is not valid");

	if(IS_BRANCH(root)) {
		struct bt_page page = btree_page_branch_find(root, key);
		return btree_find(page, key);
	} else {
		struct bt_cur cur;
		cur.page = root;
		cur.index = btree_page_leaf_find(root, key);
		return cur;
	}
}

// Add key to tree and return pointer to new root node
struct bt_page btree_add(struct bt_page root, bt_key key, bt_val val)
{
	check(root.s, "store is NULL");
	check(root.ptr, "root is not valid");

	struct bt_page orig = btree_page_null(root);
	orig.ptr = 0;
	struct bt_page split = btree_page_null(root);
	split.ptr = 0;
	bt_key mid = btree_recur_add(root, key, val, &orig, &split);

	if(split.ptr != 0) {
		debug("Root needs to make like a tree and split :[");
		orig = btree_root_create(root.s, mid, orig, split);
		store_free(root.s, root.ptr);
	}
	
	return orig;

error:
	return root;
}

// Print out a btree for debugging
static void btree_debug_page_print(struct bt_page page, int n, int level, int opts)
{
	const char *prefix = IS_ROOT(page)? "root " : "";
	const char *type = IS_LEAF(page)? "leaf" : "branch";

	printf("%s%s page %i (depth %i) at %llu: ", prefix, type, n, level, page.ptr);
	printf("size: %u, used: %u, free: %u\n", page.sz_page, page.end, page.sz_free);

	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	item_p item = page.start;
	if( IS_BRANCH(page) ) {
		// Recurse
		item += sizeof(bt_val);
		while(item < page.end) {
			bt_key key = read_ui64(page, item);
			printf("\t%llu", key);
			item += item_size;
		}
		printf("\n");
	} else {
		if(opts & BTREE_DEBUG_FULL) {
			while(item < page.end) {
				bt_key key = read_ui64(page, item);
				bt_val val = read_ui64(page, item + 8);
				printf("\t%llu:%llu", key, val);
				item += item_size;
			}
			printf("\n");
		}
	}

	if( IS_BRANCH(page) ) {
		// Recurse
		item = page.start;
		bt_val ptr = read_ui64(page, item);
		btree_debug_page_print(btree_page(page.s, ptr), ++n, level+1, opts);
		item += sizeof(bt_val);
		while(item < page.end) {
			ptr = read_ui64(page, item + 8);
			btree_debug_page_print(btree_page(page.s, ptr), ++n, level+1, opts);
			item += item_size;
		}
	}
}

void btree_debug_print(struct bt_page root, int opts)
{
	// Test
	
	root = btree_add(root, 1, 1);
	root = btree_add(root, 4, 1000);
	root = btree_add(root, 5, 10);
	root = btree_add(root, 2, 100);
	//root = btree_add(root, 3, 10000);

	// Metadata
	printf("\n");
	printf("btree in %s\n", store_desc(root.s));

	btree_debug_page_print(root, 0, 0, opts);
	printf("\n");
}
