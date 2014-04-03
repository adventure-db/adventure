/*
> folder {name}
> nodes.0 (size)
> edges.0

wmmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat st = {0};

if (stat("/some/directory", &st) == -1) {
    mkdir("/some/directory", 0700);
}
