#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "errno.h"

#include "wmmap.h"

void *wmmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	return mmap(addr, len, prot, flags, fd, offset);
}

void *wmmapfile(const char *filename)
{
	struct stat sb;
	errno = 0;
	int fd = open(filename, O_RDONLY);
	if(errno) goto fileError;

	fstat(fd, &sb);
	if(!sb.st_size) goto fileError;

	printf("Size: %llu\n", (uint64_t)sb.st_size);

	void *block = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	return block;

	fileError:
		return NULL;
}