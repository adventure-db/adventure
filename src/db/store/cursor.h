#ifndef ADV_DB_STORE_CURSOR_H
#define ADV_DB_STORE_CURSOR_H

#include "store.h"

/*	Cursor into a store-based data structure
*/

#define CURSOR_MAX_DEPTH		32

struct cursor
{
	struct store *s;
	page_p root;

	// These methods are implemented by each data structure
	struct cursor_fn
	{
		void (*get)(struct cursor *);
		void (*set)(struct cursor *, void *data);
		void (*add)(struct cursor *, void *data);
		void (*del)(struct cursor *);

		int (*first)(struct cursor *);
		int (*prev)(struct cursor *);
		int (*next)(struct cursor *);
		int (*last)(struct cursor *);
	} fn;

	page_p page[CURSOR_MAX_DEPTH];
	item_p index[CURSOR_MAX_DEPTH];
	uint64_t hdr[CURSOR_MAX_DEPTH];
	unsigned short depth;
	unsigned short top;
};

inline void cursor_get(struct cursor *cur)
{
	return cur->fn.get(cur);
}

inline void cursor_set(struct cursor *cur, void *data)
{
	cur->fn.set(cur, data);
}

inline int cursor_first(struct cursor *cur)
{
	return cur->fn.first(cur);
}

inline int cursor_prev(struct cursor *cur)
{
	return cur->fn.prev(cur);
}

inline int cursor_next(struct cursor *cur)
{
	return cur->fn.next(cur);
}

inline int cursor_last(struct cursor *cur)
{
	return cur->fn.last(cur);
}

// Generic cursor methods
inline void cursor_push(struct cursor *cur, page_p page, uint64_t hdr, item_p index)
{
	if(cur->depth) cur->top++;
	cur->page[cur->top] = page;
	cur->index[cur->top] = index;
	cur->hdr[cur->top] = hdr;
	cur->depth++;
}

inline void cursor_pop(struct cursor *cur)
{
	if(cur->depth > 0) {
		cur->depth--;
		cur->top--;
	}
}

inline void cursor_clear(struct cursor *cur)
{
	cur->depth = 0;
	cur->top = 0;
}

// Debugging
inline void cursor_debug(struct cursor *cur)
{
	for (int i=cur->top; i>=0; i--) {
		printf("*(%llu), %hu\n", cur->page[i], cur->index[i]);
	}
}

#endif
