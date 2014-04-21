#ifndef ADV_DB_STORE_BVEC_H
#define ADV_DB_STORE_BVEC_H

#include <stdint.h>
#include "store.h"

/*	Persistent bitmapped vector trie

	Features:
	-	Simple. Does not support variable-length keys or values.

	-	All inserts, updates and deletes result in a copy of the tree path
		and a new root node.

		Allocation of pages is the responsibility of the backing store.
		This gives the backing store flexibility in determining whether to
		do append-only writes or reclaim free space (or even be adaptive and
		choose its strategy based on the write load)

	-	Very lightweight with no stdlib memory allocation.
		Designed for applications that could potentially have millions of trees
		in a single file.

	-	Does not handle meta pages. It is the responsibility of the user of the
		vector to maintain a reference to the root.

	Uses in adventure:
	-	Node list
	-	Edge lists
*/

typedef uint64_t bv_val;
typedef uint64_t bv_header;

struct bv_cur
{
	uint64_t page;
	uint16_t index;
};

page_p bvec_create(struct store *s);
void bvec_destroy(struct store *s, page_p root);

page_p bvec_set(struct store *s, page_p root, uint64_t index, bv_val val);
struct bv_cur bvec_get(struct store *s, page_p root, uint64_t index);

page_p bvec_push(struct store *s, page_p root, bv_val val);
page_p bvec_pop(struct store *s, page_p root, bv_val *result);

page_p bvec_add(struct store *s, page_p root, uint64_t index, bv_val val);
page_p bvec_remove(struct store *s, page_p root, uint64_t index);

#endif
