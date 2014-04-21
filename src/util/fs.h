#ifndef ADV_UTIL_FS_H
#define ADV_UTIL_FS_H

#include <stdlib.h>
#include <lib/sds/sds.h>

// NOTE: platform-dependent code

// File I/O
int fs_create_dir(const char *path);
int fs_create_file(const char *path, size_t size);
int fs_resize_file(const char *path, size_t size);

int fs_exists(const char *path);

// Memory-mapped I/O
struct fs_map
{
	sds path;
	void *data;
	size_t size;
};

struct fs_map *fs_map(const char *path);
void fs_unmap(struct fs_map *file);
void fs_remap(struct fs_map *file);

#endif
