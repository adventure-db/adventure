#include "bvec.h"

#include <src/util/dbg.h>

#define BVEC_DEF_POWER_TWO		1
#define BVEC_DEF_PAGE_SIZE		64
#define BVEC_HEADER_SIZE		8

// Some limits for error checking
#define BVEC_MIN_PAGE_SIZE		64
#define BVEC_MAX_PAGE_SIZE		65536

#define PG_HEADER(s,page)		(read_ui64(s, page, 0))

#define PG_FLAGS(hdr)			((hdr >> 56) & 0xFFFF)
#define PG_SIZE(hdr)			((hdr >> 32) & 0xFFFF)
#define PG_NKEYS(hdr)			((hdr >> 16) & 0xFFFF)
#define PG_USED(hdr)			((hdr >> 0) & 0xFFFF)

#define PG_END(hdr)				(PG_USED(hdr))
#define PG_FREE(hdr)			(PG_SIZE(hdr) - PG_USED(hdr))
#define PG_START(hdr)			(BVEC_HEADER_SIZE)

// INLINE HELPERS
/*
static inline uint64_t read_ui64(struct store *s, page_p page, item_p i)
{
	return store_read_ui64(s, page + i);
}
*/

static inline void write_ui64(struct store *s, page_p page, item_p i, uint64_t n)
{
	store_write_ui64(s, page + i, n);
}
/*
static inline void copy(struct store *s, page_p dest, item_p di,
						page_p src, item_p si, item_p sz)
{
	store_copy(s, dest + di, src + si, sz);
}
*/
// HEADER HELPERS
static inline void bvec_hdr_write(struct store *s, page_p page,
								   unsigned char flags,
								   uint16_t sz_page,
								   uint16_t n_keys,
								   uint16_t used,
								   uint64_t next_ptr)
{
	unsigned char unused = 16;
	bv_header hdr = ((uint64_t) flags << 56) |
					((uint64_t) unused << 48) |
					((uint64_t) sz_page << 32) |
					((uint64_t) n_keys << 16) |
					((uint64_t) used << 0);
	write_ui64(s, page, 0, hdr);
}

// ITERATION HELPERS

// EXTERNAL API
page_p bvec_create(struct store *s)
{
	page_p page = store_alloc(s, BVEC_DEF_PAGE_SIZE);
	bvec_hdr_write(s, page, 0,
							BVEC_DEF_PAGE_SIZE,
							0,
							BVEC_HEADER_SIZE,
							0);
	return page;
}

// TODO: implement
void bvec_destroy(struct store *s, page_p page)
{
}

// Get an item from the vector
struct bv_cur bvec_get(struct store *s, page_p page, uint64_t index)
{
	return (struct bv_cur) {.page = 0, .index = 0};
}

// Add an item to the vector
page_p bvec_add(struct store *s, page_p page, uint64_t index, bv_val val)
{
	check(s, "store is NULL");
	check(page, "page is not valid");

error:
	return page;
}
