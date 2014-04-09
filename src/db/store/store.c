#include "store.h"

#include <src/util/dbg.h>
#include <src/util/fs.h>

#define STORE_INIT_FILE_SIZE 64
#define STORE_HEADER_SIZE 32

// Read/write file header
static void store_write_hdr(struct store *s)
{
	debug("Writing header.");

	store_p p = 0;
	store_write(s, p, "ADVDB!", 6);
	store_write_ui64(s, p+6, STORE_HEADER_SIZE);
}

static void store_read_hdr(struct store *s)
{
	store_p p = 0;
	memcpy(s->magic, s->file->data + p, 6);
	s->end = store_read_ui64(s, p+6);

	debug("Header:");
	debug("\tMagic: %s", s->magic);
	debug("\tEnd: %llu", s->end);
}

// Open/close file
struct store *store_open(const char *name, int flags)
{
	int code = -1;
	if(flags & STORE_O_CREAT) {
		code = fs_create_file(name, STORE_INIT_FILE_SIZE);
	}

	struct store *s = calloc(1, sizeof(struct store));
	check_mem(s);

	s->file = fs_map(name);
	check(s->file, "Could not open file");

	if(flags & STORE_O_CREAT) {
		if(code == 0) store_write_hdr(s);
	}
	store_read_hdr(s);

	return s;

error:
	return NULL;
}

void store_close(struct store *s)
{
	if(s) {
		fs_unmap(s->file);
		free(s);
	}
}

sds store_desc(struct store *s)
{
	return s->file->path;
}

void store_delete(struct store *s)
{
	// TODO: delete file
}

void store_repair(struct store *s)
{
	// TODO: repair file
}

// Resize file
int store_resize(struct store *s, size_t size)
{
	if(size < s->file->size) {
		// Shrink: no-op for now
	} else {
		// Expand
		fs_resize_file(s->file->path, size);
		fs_remap(s->file);
	}
	return 0;
}

int store_extend(struct store *s)
{
	return store_resize(s, s->file->size * 2);
}

// Allocator
// TODO: make this a lot smarter
// TODO: also make it safe (don't keep trying to allocate)
store_p store_alloc(struct store *s, size_t size)
{
	size_t rem_space = s->file->size - s->end;

	if(size > rem_space) {
		store_extend(s);
		return store_alloc(s, size);
	}

	size_t end = s->end;
	s->end += size;
	store_write_ui64(s, 6, s->end);
	return end;
}

// TODO: right now this is a no-op (allocated space never freed)
void store_free(struct store *s, store_p p)
{
	return;
}

// TODO: inline these functions
void store_write(struct store *s, store_p p, void *data, size_t size)
{
	check(s->file->size > s->end + size, "Trying to write beyond allocated space");
	memcpy(s->file->data + p, data, size);

error:
	return;
}

void store_write_ui16(struct store *s, store_p p, uint16_t n)
{
	char *ptr = (char *) (s->file->data + p);
	ptr[0] = n >> 8;
	ptr[1] = n;
}

void store_write_ui32(struct store *s, store_p p, uint32_t n)
{
	char *ptr = (char *) (s->file->data + p);
	ptr[0] = n >> 24;
	ptr[1] = n >> 16;
	ptr[2] = n >> 8;
	ptr[3] = n;
}

void store_write_ui64(struct store *s, store_p p, uint64_t n)
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

void *store_read(struct store *s, store_p p, size_t size)
{
	return s->file->data + p;
}

uint16_t store_read_ui16(struct store *s, store_p p)
{
	unsigned char *ptr = (unsigned char *) (s->file->data + p);
	return	(ptr[0] << 8) |
			(ptr[1] << 0);
}

uint32_t store_read_ui32(struct store *s, store_p p)
{
	unsigned char *ptr = (unsigned char *) (s->file->data + p);
	return	(ptr[0] << 24) |
			(ptr[1] << 16) |
			(ptr[2] << 8) |
			(ptr[3] << 0);
}

uint64_t store_read_ui64(struct store *s, store_p p)
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

void store_copy(struct store *s, store_p di, store_p si, size_t size)
{
	memmove(s->file->data + di, s->file->data + si, size);
}
