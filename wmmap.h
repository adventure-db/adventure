#ifndef ADV_WMMAP_H
#define ADV_WMMAP_H

void *wmmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void *wmmapfile(const char *filename);

#endif	/* ADV_WMMAP_H */