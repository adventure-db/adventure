#ifndef ADV_DB_STORE_STORE
#define ADV_DB_STORE_STORE

#include <stdint.h>
#include <lib/sds/sds.h>
#include <src/util/fs.h>

#define STORE_O_CREAT	0x1

typedef uint64_t store_p;

struct store;

struct store_alloc
{
	store_p (*alloc)(struct store *, size_t);
	store_p (*free)(struct store *, size_t);
};

struct store
{
	char magic[6];
	uint64_t end;
	struct fs_map *file;

	struct store_alloc alloc;
};

struct store *store_open(const char *path, int flags);
void store_close(struct store *s);

sds store_desc(struct store *s);
void store_delete(struct store *s);
void store_repair(struct store *s);

int store_resize(struct store *s, size_t size);
int store_extend(struct store *s);

int store_sync(struct store *s);

store_p store_alloc(struct store *s, size_t size);
void store_free(struct store *s, store_p p);

void store_write(struct store *s, store_p p, void *data, size_t size);
void store_write_ui16(struct store *s, store_p p, uint16_t n);
void store_write_ui32(struct store *s, store_p p, uint32_t n);
void store_write_ui64(struct store *s, store_p p, uint64_t n);

void *store_read(struct store *s, store_p p, size_t size);
uint16_t store_read_ui16(struct store *s, store_p p);
uint32_t store_read_ui32(struct store *s, store_p p);
uint64_t store_read_ui64(struct store *s, store_p p);

void store_copy(struct store *s, store_p di, store_p si, size_t size);

#endif
