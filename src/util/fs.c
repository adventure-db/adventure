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

int fs_create_file(const char *path, size_t size)
{
	errno = 0;
	int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
	check_debug(errno == 0, "Could not create file");
	ftruncate(fd, size);
	close(fd);
	return 0;

error:
	if(fd) close(fd);
	return errno;
}

void *fs_map_file(const char *path)
{
	struct stat sb;
	errno = 0;
	int fd = open(path, O_RDONLY);
	if(errno) goto error;

	fstat(fd, &sb);
	if(!sb.st_size) goto error;

	printf("Size: %llu\n", (uint64_t)sb.st_size);

	void *block = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	check_mem(block);
	close(fd);

	return block;

error:
	if(fd) close(fd);
	return NULL;
}
