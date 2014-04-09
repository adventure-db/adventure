#include "btree.h"

#include <src/util/dbg.h>

/*
#define BTREE_PAGE_SIZE			4096
*/

// TODO: make page size dynamic based on meta page
#define BTREE_PAGE_SIZE 		64
#define BTREE_HEADER_SIZE		8

#define BTREE_FLAG_BRANCH		0x00
#define BTREE_FLAG_LEAF			0x01

/* HEADER HELPERS */
struct bt_pg_hdr
{
	uint32_t flags;
	uint32_t is_leaf;
	uint16_t n_keys;
	uint16_t end;
	uint16_t free;
	uint16_t start;
	uint16_t page_sz;
};

static inline void btree_page_hdr_write(struct store *s, store_p page, uint32_t flags, uint16_t n_keys, uint16_t used)
{
	store_write_ui32(s, page, flags);
	store_write_ui16(s, page + 4, n_keys);
	store_write_ui16(s, page + 6, used);
}

static inline void btree_page_hdr_read(struct store *s, store_p page, struct bt_pg_hdr *hdr)
{
	hdr->flags = store_read_ui32(s, page);
	hdr->is_leaf = hdr->flags & BTREE_FLAG_LEAF;
	hdr->n_keys = store_read_ui16(s, page + 4);
	hdr->end = store_read_ui16(s, page + 6);
	hdr->free = BTREE_PAGE_SIZE - hdr->end;
	hdr->start = BTREE_HEADER_SIZE;
	hdr->page_sz = BTREE_PAGE_SIZE;
}

static uint32_t btree_page_flags(struct store *s, store_p page)
{
	return store_read_ui32(s, page);
}

static uint16_t btree_page_key_count(struct store *s, store_p page)
{
	return store_read_ui16(s, page + 4);
}

static uint16_t btree_page_size_used(struct store *s, store_p page)
{
	return store_read_ui16(s, page + 6);
}

static uint16_t btree_page_size_free(struct store *s, store_p page)
{
	return BTREE_PAGE_SIZE - btree_page_size_used(s, page);
}

/* PAGE HELPERS */
// TODO: make this a binary search
static store_p btree_page_branch_find(struct store *s, store_p page, bt_key key)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	uint32_t start = BTREE_HEADER_SIZE;
	uint32_t end = btree_page_size_used(s, page);

	store_p item = start;
	bt_val val = store_read_ui64(s, page + item);
	item += sizeof(bt_val);
	while(item < end) {
		if(key <= store_read_ui64(s, page + item)) break;
		val = store_read_ui64(s, page + item + 8);
		item += item_size;
	}
	return val;
}

// Look for key in page and return pointer to first item <= key
// TODO: make this a binary search
static store_p btree_page_leaf_find(struct store *s, store_p page, bt_key key)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	uint32_t start = BTREE_HEADER_SIZE;
	uint32_t end = btree_page_size_used(s, page);

	store_p item = start;
	while(item < end) {
		if(key <= store_read_ui64(s, page + item)) break;
		item += item_size;
	}
	return item;
}

// Create a new root branch
static store_p btree_root_create(struct store *s, bt_key mid, store_p p1, store_p p2)
{
	store_p root = store_alloc(s, BTREE_PAGE_SIZE);
	uint32_t item_size = sizeof(bt_key) + 2*sizeof(bt_val);
	uint32_t start = BTREE_HEADER_SIZE;

	btree_page_hdr_write(s, root, BTREE_FLAG_BRANCH, 1, start + item_size);
	store_write_ui64(s, root + start, p1);
	store_write_ui64(s, root + start + sizeof(bt_val), mid);
	store_write_ui64(s, root + start + sizeof(bt_val) + sizeof(bt_key), p2);
	return root;
}

// Split a branch page
static bt_key btree_page_branch_split(struct store *s, store_p page, bt_key key, store_p *p1, store_p *p2)
{
	return 0;
}

static store_p btree_page_branch_add(struct store *s, store_p branch, store_p sub, store_p *orig, store_p *split)
{
	/*
	uint32_t used_space = btree_page_size_used(s, branch);
	uint32_t free_space = btree_page_size_free(s, branch);
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	if(free_space < item_size) return 0;

	store_p branch_copy = store_alloc(s, BTREE_PAGE_SIZE);
	store_p i = btree_page_branch_find(s, branch, mid);

	// Copy over items in leaf and set mid to new mid

	// Write to header
	uint32_t flags = btree_page_flags(s, branch);
	uint32_t n_keys = btree_page_key_count(s, branch);
	btree_page_hdr_write(s, branch_copy, flags, n_keys+1, used_space + item_size);

	store_free(s, branch);
	*/
	return branch;
}

static inline store_p
btree_page_copy_add(struct store *s, store_p dest, store_p src, struct bt_pg_hdr *hdr, store_p start, store_p end,
					uint32_t n_keys, bt_key key, bt_val val, store_p i)
{
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	dest += hdr->start;
	src += hdr->start + start;

	if((start == 0 || i > start) && i <= end) {
		n_keys++;
		i -= start;
		store_copy(s, dest, src, i);
		store_write_ui64(s, dest + i, key);
		store_write_ui64(s, dest + i + sizeof(bt_key), val);
		store_copy(s, dest + i + item_size, src + i, end - i);
	} else {
		store_copy(s, dest, src, end);
	}

	btree_page_hdr_write(s, dest - hdr->start, hdr->flags, n_keys, n_keys*item_size);
	return 0;
}

// Split a leaf page, return median to copy up tree
// TODO: remove knowledge of page structure
static bt_key btree_page_leaf_split(struct store *s, store_p leaf, bt_key key, bt_val val, store_p *orig, store_p *split)
{
	struct bt_pg_hdr hdr;
	btree_page_hdr_read(s, leaf, &hdr);

	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	// Allocate the new leaves
	*orig = store_alloc(s, BTREE_PAGE_SIZE);
	*split = store_alloc(s, BTREE_PAGE_SIZE);

	// Find middle value (median) of original leaf
	store_p i = btree_page_leaf_find(s, leaf, key) - hdr.start;
	store_p m = item_size * (hdr.n_keys / 2);
	debug("i: %llu", i);
	debug("m: %llu", m);
	bt_key mid = store_read_ui64(s, leaf + hdr.start + m);

	// Copy over data to new leaves
	uint32_t n_keys_orig = (hdr.n_keys / 2);
	uint32_t n_keys_split = hdr.n_keys - n_keys_orig;
	btree_page_copy_add(s, *orig, leaf, &hdr, 0, m, n_keys_orig, key, val, i);
	btree_page_copy_add(s, *split, leaf, &hdr, m, hdr.page_sz - m, n_keys_split, key, val, i);

/*
	store_copy(s, *orig + hdr.start, leaf + hdr.start, m);
	if(i <= m) {
		n_keys_orig++;
		store_write_ui64(s, *orig + hdr.start + m, key);
		store_write_ui64(s, *orig + hdr.start + m + sizeof(bt_key), val);
	}
	store_copy(s, *split + hdr.start, leaf + hdr.start + m, hdr.end - hdr.start - m);
	if(i > m) {
		n_keys_split++;
		store_write_ui64(s, *split + hdr.end - m, key);
		store_write_ui64(s, *split + hdr.end - m + sizeof(bt_key), val);
	}

	btree_page_hdr_write(s, *orig, hdr.flags, n_keys_orig, n_keys_orig*item_size);
	btree_page_hdr_write(s, *split, hdr.flags, n_keys_split, n_keys_split*item_size);
*/

	store_free(s, leaf);
	debug("Mid %llu", mid);
	return mid;
}

// Try to add a key to the leaf page and return new page
// If page is full, return 0
static store_p btree_page_leaf_add(struct store *s, store_p leaf, bt_key key, bt_val val)
{
	uint32_t used_space = btree_page_size_used(s, leaf);
	uint32_t free_space = btree_page_size_free(s, leaf);
	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);

	if(free_space < item_size) return 0;

	store_p leaf_copy = store_alloc(s, BTREE_PAGE_SIZE);
	store_p i = btree_page_leaf_find(s, leaf, key);

	// Copy over items in leaf and wedge in new item in the right spot
	store_copy(s, leaf_copy, leaf, i);
	store_write_ui64(s, leaf_copy + i, key);
	store_write_ui64(s, leaf_copy + i + 8, val);
	store_copy(s, leaf_copy + i + item_size, leaf + i, used_space - i);

	// Write to header
	uint32_t flags = btree_page_flags(s, leaf);
	uint32_t n_keys = btree_page_key_count(s, leaf);
	btree_page_hdr_write(s, leaf_copy, flags, n_keys+1, used_space + item_size);

	store_free(s, leaf);
	return leaf_copy;
}

// Recursively add an item to the tree
// If a split occurs:
//		orig <- new pointer to original page
//		split <- new pointer to split page (if there is one)
static bt_key btree_recur_add(struct store *s, store_p page, bt_key key, bt_val val, store_p *orig, store_p *split)
{
	uint32_t flags = btree_page_flags(s, page);
	int is_leaf_page = flags & BTREE_FLAG_LEAF;

	if(is_leaf_page) {
		*orig = btree_page_leaf_add(s, page, key, val);
		if(*orig) return key;

		debug("Need to split leaf page");
		return btree_page_leaf_split(s, page, key, val, orig, split);
	} else {
		store_p sub = btree_page_branch_find(s, page, key);
		debug("Found branch %llu", sub);

		bt_key mid = btree_recur_add(s, sub, key, val, orig, split);
		debug("mid: %llu", mid);

		store_p page_copy = btree_page_branch_add(s, page, sub, orig, split);
		*orig = page;
		if(page_copy) return mid;
		return btree_page_branch_split(s, page, mid, orig, split);
	}
}

/* EXTERNAL API */
store_p btree_alloc(struct store *s)
{
	store_p root = store_alloc(s, BTREE_PAGE_SIZE);
	btree_page_hdr_write(s, root, BTREE_FLAG_LEAF, 0, BTREE_HEADER_SIZE);
	return root;
}

// TODO: implement
void btree_free(struct store *s, store_p root)
{
}

// Find key in tree and return pointer to its location
store_p btree_find(struct store *s, store_p root, bt_key key)
{
	check(s, "store is NULL");
	check(root, "root is not valid");

	uint32_t flags = btree_page_flags(s, root);

	if(flags & BTREE_FLAG_BRANCH) {
		store_p page = btree_page_branch_find(s, root, key);
		return btree_find(s, page, key);
	} else {
		return btree_page_leaf_find(s, root, key);
	}

error:
	return 0;
}

// Add key to tree and return pointer to new root node
store_p btree_add(struct store *s, store_p root, bt_key key, bt_val val)
{
	check(s, "store is NULL");
	check(root, "root is not valid");

	store_p orig = 0, split = 0;
	bt_key mid = btree_recur_add(s, root, key, val, &orig, &split);

	if(split) {
		// Root needs to split :[
		debug("Root needs to make like a tree and split :[");
		orig = btree_root_create(s, mid, orig, split);
		store_free(s, root);
	}
	
	return orig;

error:
	return 0;
}

// Print out a btree for debugging
static void btree_debug_page_print(struct store *s, store_p page, int n, int level, int opts)
{
	uint32_t used_space = btree_page_size_used(s, page);
	uint32_t free_space = btree_page_size_free(s, page);
	uint32_t flags = btree_page_flags(s, page);
	int is_leaf_page = flags & BTREE_FLAG_LEAF;
	const char *type = is_leaf_page? "leaf" : "branch";

	printf("%s page %i (depth %i) at %llu: ", type, n, level, page);
	printf("used: %u, free: %u\n", used_space, free_space);

	uint32_t item_size = sizeof(bt_key) + sizeof(bt_val);
	uint32_t start = BTREE_HEADER_SIZE;
	store_p item = start;

	if(!is_leaf_page) {
		// Recurse
		item += sizeof(bt_val);
		printf("\tkeys ");
		while(item < used_space) {
			bt_key key = store_read_ui64(s, page + item);
			printf("%llu ", key);
			item += item_size;
		}
		printf("\n");
	} else {
		if(opts & BTREE_DEBUG_FULL) {
			//printf("\t");
			while(item < used_space) {
				bt_key key = store_read_ui64(s, page + item);
				bt_val val = store_read_ui64(s, page + item + 8);
				printf("\t%llu:%llu", key, val);
				item += item_size;
			}
			printf("\n");
		}
	}

	if(!is_leaf_page) {
		// Recurse
		item = start;
		bt_val val = store_read_ui64(s, page + item);
		btree_debug_page_print(s, val, ++n, level+1, opts);
		item += sizeof(bt_val);
		while(item < used_space) {
			val = store_read_ui64(s, page + item + 8);
			btree_debug_page_print(s, val, ++n, level+1, opts);
			item += item_size;
		}
	}
}

void btree_debug_print(struct store *s, store_p root, int opts)
{
	// Test
	root = btree_add(s, root, 1, 1);
	root = btree_add(s, root, 4, 1000);
	root = btree_add(s, root, 5, 10);
	root = btree_add(s, root, 2, 100);
	//root = btree_add(s, root, 3, 10000);

	// Metadata
	printf("\n");
	printf("btree in %s\n", store_desc(s));

	btree_debug_page_print(s, root, 0, 0, opts);
	printf("\n");
}

