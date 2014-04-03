#ifndef ADV_UTIL_FS_H
#define ADV_UTIL_FS_H

#include <stdlib.h>
// NOTE: platform-dependent code

int fs_create_dir(const char *path);
int fs_create_file(const char *path, size_t size);

void *fs_map_file(const char *path);

#endif
