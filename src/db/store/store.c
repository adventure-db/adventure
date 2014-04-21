#include "store.h"
#include "btree.h"

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
	int exists = fs_exists(name);
	check(exists || (flags & STORE_O_CREAT), "File does not exist");

	if (!exists && flags & STORE_O_CREAT) {
		code = fs_create_file(name, STORE_INIT_FILE_SIZE);
		check(code == 0, "Could not create file");
	}

	struct store *s = calloc(1, sizeof(struct store));
	check_mem(s);

	s->file = fs_map(name);
	check(s->file, "Could not open file");

	if(!exists && (flags & STORE_O_CREAT)) {
		store_write_hdr(s);
		store_read_hdr(s);
		s->free_root = btree_create(s);
	} else {
		store_read_hdr(s);
	}

	s->free_list = list_create();

	return s;

error:
	return NULL;
}

void store_close(struct store *s)
{
	if(s) {
		fs_unmap(s->file);
		if(s->free_list) list_destroy(s->free_list);
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
// TODO error checking
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

// Page allocation
page_p store_alloc(struct store *s, size_t size)
{
	size_t free_space = s->file->size - s->end;

	// No free space, try to extend the file
	if (size > free_space) {
		check(store_extend(s) == 0, "Could not extend file store");
		return store_alloc(s, size);
	}

	// No free pages available, return a new one from the end
	if (!s->free_list || !list_count(s->free_list)) {
		size_t end = s->end;
		s->end += size;
		store_write_ui64(s, 6, s->end);
		return end;
	}

	page_p *j = list_shift(s->free_list);
	page_p p = *j;
	debug("alloc: reusing *%llu", p);
	free(j);
	return p;

error:
	return 0;
}

void store_free(struct store *s, page_p p)
{
	page_p *j = malloc(sizeof(page_p));
	*j = p;
	list_unshift(s->free_list, j);
	debug("alloc: freeing *%llu", p);
}

/*
// Crappy linked list allocator
page_p store_alloc(struct store *s, size_t size)
{
	size_t free_space = s->file->size - s->end;

	// No free space, try to extend the file
	if (size > free_space) {
		check(store_extend(s) == 0, "Could not extend file store");
		return store_alloc(s, size);
	}

	// No free pages available, return a new one from the end
	if (!list_count(s->free_list)) {
		size_t end = s->end;
		s->end += size;
		store_write_ui64(s, 6, s->end);
		return end;
	}

	page_p *j = list_shift(s->free_list);
	page_p p = *j;
	debug("alloc: reusing *%llu", p);
	free(j);
	return p;

error:
	return 0;
}

void store_free(struct store *s, page_p p)
{
	page_p *j = malloc(sizeof(page_p));
	*j = p;
	list_unshift(s->free_list, j);
	debug("alloc: freeing *%llu", p);
}
*/

/* Simplest allocator:

// TODO: make this a lot smarter
// TODO: also make it safe (don't keep trying to allocate)
page_p store_alloc(struct store *s, size_t size)
{
	size_t free_space = s->file->size - s->end;

	if(size > free_space) {
		check(store_extend(s) == 0, "Could not extend file store");
		return store_alloc(s, size);
	}

	size_t end = s->end;
	s->end += size;
	store_write_ui64(s, 6, s->end);
	return end;

error:
	return 0;
}

// TODO: right now this is a no-op (allocated space never freed)
void store_free(struct store *s, page_p p)
{
	return;
}
*/