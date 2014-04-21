#include "bstack.h"

#include <src/util/dbg.h>

#define BSTACK_DEF_PAGE_SIZE 	64
#define BSTACK_HEADER_SIZE		16

// Some limits for error checking
#define BSTACK_MIN_PAGE_SIZE		64
#define BSTACK_MAX_PAGE_SIZE		65536

#define PG_HEADER(s,page)		(read_ui64(s, page, 0))
#define PG_NEXT(s, page)		(read_ui64(s, page, 8))

#define PG_FLAGS(hdr)			((hdr >> 56) & 0xFFFF)
#define PG_SIZE(hdr)			((hdr >> 32) & 0xFFFF)
#define PG_NKEYS(hdr)			((hdr >> 16) & 0xFFFF)
#define PG_USED(hdr)			((hdr >> 0) & 0xFFFF)

#define PG_END(hdr)				(PG_USED(hdr))
#define PG_FREE(hdr)			(PG_SIZE(hdr) - PG_USED(hdr))
#define PG_START(hdr)			(BSTACK_HEADER_SIZE)

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
static inline void bstack_hdr_write(struct store *s, page_p page,
								   unsigned char flags,
								   uint16_t sz_page,
								   uint16_t n_keys,
								   uint16_t used,
								   uint64_t next_ptr)
{
	unsigned char unused = 16;
	bs_header hdr = ((uint64_t) flags << 56) |
					((uint64_t) unused << 48) |
					((uint64_t) sz_page << 32) |
					((uint64_t) n_keys << 16) |
					((uint64_t) used << 0);
	write_ui64(s, page, 0, hdr);
	write_ui64(s, page, 8, next_ptr);
}

// ITERATION HELPERS
static void bstack_page_walk(struct store *s, page_p page,
							void (*iter)(struct store *, bs_val))
{
	bs_header hdr = PG_HEADER(s, page);
	item_p item = PG_START(hdr);

	while (item < PG_END(hdr)) {
		bs_val val = read_ui64(s, page, item);
		iter(s, val);
		item += sizeof(bs_val);
	}
}

// DEBUGGING
static inline void bstack_debug_print_item(struct store *s, bs_val val)
{
	printf("%llu ", val);
}

void bstack_debug_page(struct store *s, page_p page)
{
	bs_header hdr = PG_HEADER(s, page);

	printf("----------------------------------------\n");
	printf("bstack page (*%llu) n: %llu \n"
				 "size: %llu, used: %llu, free: %llu [%d, %llu] \n",
				 page, PG_NKEYS(hdr),
				 PG_SIZE(hdr), PG_USED(hdr), PG_FREE(hdr),
				 PG_START(hdr), PG_END(hdr));
	printf("\t");
	bstack_page_walk(s, page, bstack_debug_print_item);
	printf("\n");

	page_p next = PG_NEXT(s, page);
	if(next) bstack_debug_page(s, next);
}

// EXTERNAL API
page_p bstack_create(struct store *s)
{
	page_p page = store_alloc(s, BSTACK_DEF_PAGE_SIZE);
	bstack_hdr_write(s, page, 0,
							 BSTACK_DEF_PAGE_SIZE,
							 0,
							 BSTACK_HEADER_SIZE,
							 0);
	return page;
}

// TODO: implement
void bstack_destroy(struct store *s, page_p page)
{
}

// Find key in tree and return a cursor to its location
struct bs_cur bstack_find(struct store *s, page_p page, bs_val val)
{
	check(s, "store is NULL");
	check(page, "page is not valid");

error:
	return (struct bs_cur){.page = 0, .index = 0};
}

// Add an item to the page list
page_p bstack_add(struct store *s, page_p page, bs_val val)
{
	check(s, "store is NULL");
	check(page, "page is not valid");

	bs_header hdr = PG_HEADER(s, page);
	page_p next = PG_NEXT(s, page);
	item_p end = PG_END(hdr);

	page_p page_copy = store_alloc(s, PG_SIZE(hdr));
	if(PG_FREE(hdr) < sizeof(val)) {
		bstack_hdr_write(s, page_copy, PG_FLAGS(hdr),
									   PG_SIZE(hdr),
									   1,
									   PG_START(hdr) + sizeof(val),
									   page);
		write_ui64(s, page_copy, PG_START(hdr), val);
	} else {
		bstack_hdr_write(s, page_copy, PG_FLAGS(hdr),
									   PG_SIZE(hdr),
									   PG_NKEYS(hdr)+1,
									   PG_USED(hdr) + sizeof(val),
									   next);

		// Copy over items and append new item
		copy(s, page_copy, PG_START(hdr), page, PG_START(hdr), end);
		write_ui64(s, page_copy, end, val);
		store_free(s, page);
	}

	return page_copy;

error:
	return page;
}

// Print out a bstack for debugging
// TODO: implement
void bstack_debug_print(struct store *s, page_p root, int opts)
{
	printf("\n");
	printf("bstack in %s\n", store_desc(s));

	printf("\n");
}
