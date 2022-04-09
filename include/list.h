#pragma once
/*
 * 一份Linux list的拷贝
 */

/*
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

struct list_head {
    struct list_head *next, *prev;
};

static inline void __list_add(struct list_head *new,
        struct list_head *prev,
        struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
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
        struct list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
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
