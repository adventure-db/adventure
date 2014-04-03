#ifndef ADV_LIST_H
#define ADV_LIST_H

#include <stdlib.h>

typedef struct listelem *list_iter;

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

void list_unshift(struct list *list, void *val);
void *list_shift(struct list *list);

void *list_remove(struct list *list, struct listelem *elem);

static inline unsigned long list_count(struct list *list)
{
	return list->count;
}

static inline void *list_first(struct list *list)
{
	return list->first? list->first->val : NULL;
}

static inline void *list_last(struct list *list)
{
	return list->last? list->last->val : NULL;
}

#endif
