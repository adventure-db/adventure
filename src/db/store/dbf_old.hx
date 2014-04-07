#ifndef ADV_DB_STORE_DBF
#define ADV_DB_STORE_DBF

#include <stdint.h>
#include <lib/sds/sds.h>
#include <src/util/math.h>

#define DBF_MAX_N_FILES 100

// Open flags
#define DBF_O_CREAT	0x01

typedef uint64_t dbf_p;

struct dbf
{
	sds name;
	void *file[DBF_MAX_N_FILES];
	size_t n_files;
	size_t init_size;
};

struct dbf *dbf_open(const char *path, int flags);
void dbf_close(struct dbf *dbf);

void dbf_add_vol(struct dbf *dbf);

dbf_p dbf_alloc(struct dbf *dbf, size_t size);
void dbf_free(struct dbf *dbf, dbf_p i);

int dbf_cpy(struct dbf *dbf, dbf_p di, dbf_p si, size_t size);
int dbf_set(struct dbf *dbf, dbf_p i, size_t size);

// This operation needs to be *fast*
// TODO: remove branching
static inline void *dbf_get(struct dbf *dbf, dbf_p p)
{
	size_t l = log2(p);
	int f = l - dbf->init_size + 1;
	if(f <= 0) return dbf->file[0] + p;

	size_t i = p & ~(1 << l);
	return dbf->file[f] + i;
}

#endif
