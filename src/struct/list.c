#include "list.h"
#include <stdlib.h>
#include <src/util/dbg.h>

struct list *list_create()
{
	return calloc(1, sizeof(struct list));
}

void list_destroy(struct list *list)
{
	struct listelem *cur = NULL;
	for(cur = list->first; cur != NULL; cur = cur->next) {
		if(cur->prev) {
			free(cur->prev);
		}
	}

	free(list->last);
	free(list);
}

void list_clear(struct list *list)
{
	struct listelem *cur = NULL;
	for(cur = list->first; cur != NULL; cur = cur->next) {
		free(cur->val);
	}
}

void list_push(struct list *list, void *val)
{
	struct listelem *elem = calloc(1, sizeof(struct listelem));
	elem->val = val;

	if(list->last) {
		elem->prev = list->last;
		list->last->next = elem;
		list->last = elem;
	} else {
		list->first = elem;
		list->last = elem;
	}

	list->count++;
}

void *list_pop(struct list *list)
{
	return list_remove(list, list->last);
}

void list_unshift(struct list *list, void *val)
{
	struct listelem *elem = calloc(1, sizeof(struct listelem));
	elem->val = val;
	elem->prev = NULL;
	elem->next = list->first;

	if(list->first) {
		list->first->prev = elem;
		list->first = elem;
	} else {
		list->first = elem;
		list->last = elem;
	}
	
	list->count++;
}

void *list_shift(struct list *list)
{
	return list_remove(list, list->first);
}

void *list_remove(struct list *list, struct listelem *elem)
{
	void *val = NULL;
	check(list->first != NULL, "Cannot remove from empty list");

	if(elem == list->first && elem == list->last) {
		list->first = NULL;
		list->last = NULL;
	} else if(elem == list->last) {
		list->last = elem->prev;
		list->last->next = NULL;
	} else if(elem == list->first) {
		list->first = elem->next;
		list->first->prev = NULL;
	} else {
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}

	list->count--;
	val = elem->val;
	free(elem);

error:
	return val;
}
