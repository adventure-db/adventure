#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "errno.h"

#include <src/util/dbg.h>

int fs_create_dir(const char *path)
{
	errno = 0;
	mkdir(path, S_IRWXU);
	check_debug(errno == 0, "Could not create dir");
	return 0;

error:
	return errno;
}

int fs_exists(const char *path) {
    struct stat st;
    int rc = stat(path, &st);
    return rc == 0;
}

int fs_create_file(const char *path, size_t size)
{
	errno = 0;
	int fd = open(path, O_CREAT | O_WRONLY, S_IRWXU);
	check_debug(errno == 0, "Could not create file");
	ftruncate(fd, size);
	close(fd);
	return 0;

error:
	if(fd) close(fd);
	return errno;
}

int fs_resize_file(const char *path, size_t size)
{
	errno = 0;
	int fd = open(path, O_WRONLY, S_IRWXU);
	check(errno == 0, "Could not create file");
	ftruncate(fd, size);
	check(errno == 0, "Could not resize file");
	close(fd);
	return 0;

error:
	debug("errno = %u, %s", errno, strerror(errno));
	int code = errno;
	if(fd) close(fd);
	return code;
}

// TODO: clean this code up

static int fs_map_internal(struct fs_map *file)
{
	struct stat sb;
	errno = 0;
	int fd = open(file->path, O_RDWR);
	check(errno == 0, "Could not open file for reading");

	fstat(fd, &sb);
	check(sb.st_size, "File has no size");
	file->size = sb.st_size;

	int flags = PROT_READ | PROT_WRITE;
	file->data = mmap(NULL, file->size, flags, MAP_SHARED, fd, 0);
	check_mem(file->data);
	close(fd);

	return 0;

error:
	if(fd) close(fd);
	return -1;
}

struct fs_map *fs_map(const char *path)
{
	struct fs_map *file = calloc(1, sizeof(struct fs_map));
	file->path = sdsnew(path);
	check(fs_map_internal(file) == 0, "Could not map file");
	return file;

error:
	// Clean up on error
	if(file) {
		sdsfree(file->path);
		free(file);
	}
	return NULL;
}

void fs_unmap(struct fs_map *file)
{
	munmap(file->data, file->size);
	sdsfree(file->path);
	free(file);
}

void fs_remap(struct fs_map *file)
{
	munmap(file->data, file->size);
	fs_map_internal(file);
}
