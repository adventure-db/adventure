#include "dbf.h"

#include <src/util/dbg.h>
#include <src/util/fs.h>

#define DBF_INIT_FILE_SIZE 1024*1024
#define DBF_HEADER_SIZE 32

// Read/write file header
static void dbf_write_hdr(struct dbf *f)
{
	dbf_p p = 0;
	dbf_write(f, p, "ADVDB!", 6);
	dbf_write_ui64(f, p+6, DBF_HEADER_SIZE);
}

static void dbf_read_hdr(struct dbf *f)
{
	dbf_p p = 0;
	memcpy(f->magic, f->file->data + p, 6);
	f->end = dbf_read_ui64(f, p+6);

	printf("Magic: %s\n", f->magic);
	printf("End: %llu\n", f->end);
}

// Open/close file
struct dbf *dbf_open(const char *name, int flags)
{
	int code = -1;
	if(flags & DBF_O_CREAT) {
		code = fs_create_file(name, DBF_INIT_FILE_SIZE);
	}

	struct dbf *f = calloc(1, sizeof(struct dbf));
	check_mem(f);

	f->file = fs_map(name);
	check(f->file, "Could not open file");

	if(code == 0) dbf_write_hdr(f);
	dbf_read_hdr(f);

	return f;

error:
	return NULL;
}

void dbf_close(struct dbf *f)
{
	if(f) {
		fs_unmap(f->file);
		free(f);	
	}
}

void dbf_delete(struct dbf *f)
{
	// TODO: delete file
}

// Resize file
int dbf_resize(struct dbf *f, size_t size)
{
	if(size < f->file->size) {
		// Shrink: no-op for now
	} else {
		// Expand
		fs_resize_file(f->file->path, size);
		fs_remap(f->file);
	}
	return 0;
}

int dbf_extend(struct dbf *f)
{
	return dbf_resize(f, f->file->size * 2);
}

// Allocator
// TODO: make this a lot smarter
// TODO: also make it safe (don't keep trying to allocate)
dbf_p dbf_alloc(struct dbf *f, size_t size)
{
	size_t rem_space = f->file->size - f->end;

	if(size > rem_space) {
		dbf_extend(f);
		return dbf_alloc(f, size);
	}

	size_t end = f->end;
	f->end += size;
	dbf_write_ui64(f, 6, f->end);
	return end;
}

// TODO: right now this is a no-op (allocated space never freed)
void dbf_free(struct dbf *f, dbf_p p)
{
	return;
}

// TODO: inline these functions
void dbf_write(struct dbf *f, dbf_p p, void *data, size_t size)
{
	check(f->file->size > f->end + size, "Trying to write beyond allocated space");
	memcpy(f->file->data + p, data, size);

error:
	return;
}

void dbf_write_ui16(struct dbf *f, dbf_p p, uint16_t n)
{
	char *ptr = (char *) (f->file->data + p);
	ptr[0] = n >> 8;
	ptr[1] = n;
}

void dbf_write_ui32(struct dbf *f, dbf_p p, uint32_t n)
{
	char *ptr = (char *) (f->file->data + p);
	ptr[0] = n >> 24;
	ptr[1] = n >> 16;
	ptr[2] = n >> 8;
	ptr[3] = n;
}

void dbf_write_ui64(struct dbf *f, dbf_p p, uint64_t n)
{
	char *ptr = (char *) (f->file->data + p);
	ptr[0] = n >> 56;
	ptr[1] = n >> 48;
	ptr[2] = n >> 40;
	ptr[3] = n >> 32;
	ptr[4] = n >> 24;
	ptr[5] = n >> 16;
	ptr[6] = n >> 8;
	ptr[7] = n;
}

void *dbf_read(struct dbf *f, dbf_p p, size_t size)
{
	return f->file->data + p;
}

uint16_t dbf_read_ui16(struct dbf *f, dbf_p p)
{
	unsigned char *ptr = (unsigned char *) (f->file->data + p);
	return	(ptr[0] << 8) |
			(ptr[1] << 0);
}

uint32_t dbf_read_ui32(struct dbf *f, dbf_p p)
{
	unsigned char *ptr = (unsigned char *) (f->file->data + p);
	return	(ptr[0] << 24) |
			(ptr[1] << 16) |
			(ptr[2] << 8) |
			(ptr[3] << 0);
}

uint64_t dbf_read_ui64(struct dbf *f, dbf_p p)
{
	unsigned char *ptr = (unsigned char *) (f->file->data + p);
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

void dbf_copy(struct dbf *f, dbf_p di, dbf_p si, size_t size)
{
	memmove(f->file->data + di, f->file->data + si, size);
}
