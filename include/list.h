#pragma once
/*
 * 一份Linux list的拷贝
 */

/*
LIST_HEAD_INIT
LIST_HEAD
INIT_LIST_HEAD
__list_add
list_add
list_add_tail
__list_del
list_del
list_replace
list_empty
list_entry
list_for_each
list_for_each_prev
list_for_each_entry
list_for_each_entry_reverse
*/

#include <stddef.h>
#include <stdbool.h>

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *_new,
        struct list_head *prev,
        struct list_head *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

static inline void list_add(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head, head->next);
}

static inline void list_add_tail(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_replace(struct list_head *old,
        struct list_head *_new)
{
    _new->next = old->next;
    _new->next->prev = _new;
    _new->prev = old->prev;
    _new->prev->next = _new;
}

static inline int list_empty(const struct list_head *head)
{
    if ( head->next == head )
    {
        if ( head->prev != head )
        {
            __builtin_unreachable();
        }
        return true;
    }
    if ( head->prev == head )
    {
        __builtin_unreachable();
    }
    return false;
}

#define list_entry(ptr, type, member) \
    ((type *)((size_t)(ptr) - offsetof(type, member)))

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_entry(pos, head, member)				\
    for (pos = list_entry((head)->next, __typeof__(*pos), member);	\
            &pos->member != (head);			\
            pos = list_entry((pos)->member.next, __typeof__(*(pos)), member))

#define list_for_each_entry_reverse(pos, head, member)			\
    for (pos = list_entry((head)->prev, __typeof__(*pos), member);		\
            &pos->member != (head); 			\
            pos = list_entry((pos)->member.prev, __typeof__(*(pos)), member))
