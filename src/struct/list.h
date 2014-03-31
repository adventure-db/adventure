#ifndef ADV_LIST_H
#define ADV_LIST_H

struct listelem
{
	struct listelem *prev;
	struct listelem *next;
	void *val;
};

struct list
{
	unsigned long count;
	struct listelem *first;
	struct listelem *last;
};

struct list *list_create();
void list_destroy(struct list *list);
void list_clear(struct list *list);

void list_push(struct list *list, void *val);
void *list_pop(struct list *list);

void *list_remove(struct list *list, struct listelem *elem);

#endif