#ifndef ADV_DB_STORE_BSTACK_H
#define ADV_DB_STORE_BSTACK_H

#include <stdint.h>
#include "store.h"

/*	Copy-on-write page stack

	Features:
	-	Simple. Does not support variable-length values.
	-	O(1) immutable push/pop from front of stack

	Uses in adventure:
	-	Overflow pages
*/

typedef uint64_t bs_val;
typedef uint64_t bs_header;

// Cursor to a specific item
struct bs_cur
{
	page_p page;
	item_p index;
};

page_p bstack_create(struct store *s);
void bstack_destroy(struct store *s, page_p page);

page_p bstack_add(struct store *s, page_p page, bs_val val);
struct bs_cur bstack_find(struct store *s, page_p page, bs_val val);
page_p bstack_remove(struct store *s, page_p page, bs_val val);

void bstack_debug_print(struct store *s, page_p page, int opts);
void bstack_debug_page(struct store *s, page_p page);

#endif
