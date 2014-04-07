#ifndef ADV_DB_STORE_DBF
#define ADV_DB_STORE_DBF

#include <stdint.h>
#include <lib/sds/sds.h>
#include <src/util/fs.h>

#define DBF_O_CREAT	0x1

typedef uint64_t dbf_p;

struct dbf
{
	char magic[6];
	uint64_t end;
	struct fs_map *file;
};

struct dbf *dbf_open(const char *path, int flags);
void dbf_close(struct dbf *f);
void dbf_delete(struct dbf *f);

int dbf_resize(struct dbf *f, size_t size);
int dbf_extend(struct dbf *f);

dbf_p dbf_alloc(struct dbf *f, size_t size);
dbf_p dbf_realloc(struct dbf *f, dbf_p p, size_t size);
void dbf_free(struct dbf *f, dbf_p p);

void dbf_write(struct dbf *f, dbf_p p, void *data, size_t size);
void dbf_write_ui16(struct dbf *f, dbf_p p, uint16_t n);
void dbf_write_ui32(struct dbf *f, dbf_p p, uint32_t n);
void dbf_write_ui64(struct dbf *f, dbf_p p, uint64_t n);

void *dbf_read(struct dbf *f, dbf_p p, size_t size);
uint16_t dbf_read_ui16(struct dbf *f, dbf_p p);
uint32_t dbf_read_ui32(struct dbf *f, dbf_p p);
uint64_t dbf_read_ui64(struct dbf *f, dbf_p p);

void dbf_copy(struct dbf *f, dbf_p di, dbf_p si, size_t size);

#endif
