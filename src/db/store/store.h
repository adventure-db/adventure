#ifndef ADV_DB_STORE_STORE
#define ADV_DB_STORE_STORE

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <lib/sds/sds.h>
#include <src/util/dbg.h>
#include <src/util/fs.h>
#include <src/struct/list.h>

/*	Managed data store abstraction

	In adventure this module wraps a memory-mapped file, but there's no reason
	it couldn't have a different backend (such as memory or page cache)

	Responsibilities:
	-	Allocation and release of variable-length pages
	-	Transaction support:
		-	Retaining pages that are being used by an active transaction
*/

#define STORE_O_CREAT	0x1

typedef uint64_t store_p;
typedef uint16_t item_p;
typedef store_p page_p;

struct store
{
	char magic[6];
	uint64_t end;
	struct fs_map *file;
	struct list *free_list;
	page_p free_root;
};

struct store *store_open(const char *path, int flags);
void store_close(struct store *s);

sds store_desc(struct store *s);

void store_delete(struct store *s);
void store_repair(struct store *s);

// Resizing
int store_resize(struct store *s, size_t size);
int store_extend(struct store *s);

// Synchronous writes
int store_sync(struct store *s);

// Page allocation
page_p store_alloc(struct store *s, size_t size);
void store_free(struct store *s, page_p p);

// Inlined read/write methods
inline void store_write(struct store *s, store_p p, void *data, size_t size)
{
	check(s->file->size > s->end + size, "Trying to write beyond allocated space");
	memcpy(s->file->data + p, data, size);

error:
	return;
}

inline void store_write_ui16(struct store *s, store_p p, uint16_t n)
{
	char *ptr = (char *) (s->file->data + p);
	ptr[0] = n >> 8;
	ptr[1] = n;
}

inline void store_write_ui32(struct store *s, store_p p, uint32_t n)
{
	char *ptr = (char *) (s->file->data + p);
	ptr[0] = n >> 24;
	ptr[1] = n >> 16;
	ptr[2] = n >> 8;
	ptr[3] = n;
}

inline void store_write_ui64(struct store *s, store_p p, uint64_t n)
{
	unsigned char *ptr = (unsigned char *) (s->file->data + p);
	ptr[0] = n >> 56;
	ptr[1] = n >> 48;
	ptr[2] = n >> 40;
	ptr[3] = n >> 32;
	ptr[4] = n >> 24;
	ptr[5] = n >> 16;
	ptr[6] = n >> 8;
	ptr[7] = n;
}

inline void *store_read(struct store *s, store_p p, size_t size)
{
	return s->file->data + p;
}

inline uint16_t store_read_ui16(struct store *s, store_p p)
{
	unsigned char *ptr = (unsigned char *) (s->file->data + p);
	return	(ptr[0] << 8) |
			(ptr[1] << 0);
}

inline uint32_t store_read_ui32(struct store *s, store_p p)
{
	unsigned char *ptr = (unsigned char *) (s->file->data + p);
	return	(ptr[0] << 24) |
			(ptr[1] << 16) |
			(ptr[2] << 8) |
			(ptr[3] << 0);
}

inline uint64_t store_read_ui64(struct store *s, store_p p)
{
	unsigned char *ptr = (unsigned char *) (s->file->data + p);
	uint64_t p0 = ptr[0];
	uint64_t p1 = ptr[1];
	uint64_t p2 = ptr[2];
	uint64_t p3 = ptr[3];
	uint64_t p4 = ptr[4];
	uint64_t p5 = ptr[5];
	uint64_t p6 = ptr[6];
	uint64_t p7 = ptr[7];
	return	(p0 << 56) | (p1 << 48) | (p2 << 40) | (p3 << 32) |
			(p4 << 24) | (p5 << 16) | (p6 << 8) | (p7 << 0);
}

inline void store_copy(struct store *s, store_p di, store_p si, size_t size)
{
	if(size > 0) {
		memcpy(s->file->data + di, s->file->data + si, size);
	}
}

#endif
