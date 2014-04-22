#include "btree.h"
#include "bstack.h"

#include <src/util/dbg.h>

#define BTREE_DEF_PAGE_SIZE 	64
#define BTREE_HEADER_SIZE		8

#define BTREE_ROOT 				1
#define BTREE_LEAF				2
#define BTREE_DUPS				4

// Some limits for error checking
#define BTREE_MIN_PAGE_SIZE		64
#define BTREE_MAX_PAGE_SIZE		65536

#define PG_HEADER(s,page)		(read_ui64(s, page, 0))

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

#define CUR_PG_START(cur)		(PG_START(cur->hdr[cur->top]))
#define CUR_PG_END(cur)			(PG_END(cur->hdr[cur->top]))
#define CUR_LEAF(cur)			(IS_LEAF(cur->hdr[cur->top]))
#define CUR_BRANCH(cur)			(IS_BRANCH(cur->hdr[cur->top]))
#define CUR_PAGE(cur)			(cur->page[cur->top])
#define CUR_INDEX(cur)			(cur->index[cur->top])
#define CUR_HDR(cur)			(cur->hdr[cur->top])
#define CUR_KEY(cur)			()

void btree_cursor_find(struct cursor *cur, bt_key key)
{
	if (CUR_BRANCH(cur)) {
		item_p index = btree_branch_find(cur->s, CUR_PAGE(cur), key);
		page_p sub = read_ui64(cur->s, CUR_PAGE(cur), index);
		bt_header hdr = PG_HEADER(cur->s, sub);
		cursor_push(cur, sub, hdr, index);
		btree_cursor_find(cur, key);
	} else {
		item_p index = btree_leaf_find(cur->s, CUR_PAGE(cur), key);
		cursor_push(cur, CUR_PAGE(cur), CUR_HDR(cur), index);
	}
}

void btree_cursor_add(struct cursor *cur, bt_key key, bt_val val)
{
	cursor_clear(cur);
	while (cur->depth < CURSOR_MAX_DEPTH) {
		page_p page = 0;
		if (!cur->depth) {
			page = cur->root;
		} else {

		}

		bt_header hdr = PG_HEADER(cur->s, page);
		cursor_push(cur, page, hdr, PG_START(hdr));

		if (CUR_LEAF(cur)) {
			//CUR_PAGE(cur) = btree_leaf_add(cur->s, CUR_PAGE(cur), key, val);
			return;
		}
	}
}

static void btree_cursor_index(struct store *s, page_p page,
							   struct cursor *cur, item_p index)
{
	bt_header hdr = PG_HEADER(s, page);

	size_t offset = 0;
	if (IS_BRANCH(hdr) && PG_NKEYS(hdr)) offset = sizeof(page_p);

	item_p i = index;
	if(i == 0) i = PG_START(hdr);
	else if(i == ITEM_MAX) i = PG_END(hdr) - offset;

	cursor_push(cur, page, hdr, i);

	if (!IS_LEAF(hdr)) {
		page_p sub = read_ui64(s, page, i);
		btree_cursor_index(s, sub, cur, index);
	}
}

void btree_cursor_first(struct store *s, page_p page, struct cursor *cur)
{
	cursor_clear(cur);
	btree_cursor_index(s, page, cur, 0);
}

void btree_cursor_last(struct store *s, page_p page, struct cursor *cur)
{
	cursor_clear(cur);
	btree_cursor_index(s, page, cur, ITEM_MAX);
}

static int btree_cursor_prev(struct cursor *cur)
{
	return 0;
}

static struct cursor_fn btree_cursor_fn = (struct cursor_fn) {
	.prev = btree_cursor_prev
};

struct cursor btree_cursor(struct store *s, page_p root)
{
	return (struct cursor)
	{
		.s = s,
		.root = root,
		.depth = 0,
		.top = 0,
		.fn = btree_cursor_fn
	};
}

int btree_cursor_next(struct store *s, struct cursor *cur)
{
	if(cur->depth == 0) return -1;

	page_p page = cur->page[cur->depth - 1];
	item_p index = cur->index[cur->depth - 1];
	bt_header hdr = PG_HEADER(s, page);
	size_t item_size = sizeof(page_p) + sizeof(bt_key);
	if (index + item_size >= PG_END(hdr)) {
		cur->depth--;
		if (btree_cursor_next(s, cur)) {
			cur->depth++;
			return -1;
		}
		cur->page[cur->depth] = read_ui64(s, cur->page[cur->depth-1], cur->index[cur->depth-1]);
		cur->index[cur->depth] = 0;
		cur->depth++;
	} else {
		cur->index[cur->depth-1] += item_size;
	}
	return 0;
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

void btree_debug(struct store *s, page_p root, int opts)
{
	printf("btree in %s\n", store_desc(s));
	btree_debug_recur(s, root, 0, 0, opts);
}

// Predicates
bool btree_is_empty(struct store *s, page_p page)
{
	bt_header hdr = PG_HEADER(s, page);
	return PG_NKEYS(hdr) == 0;
}