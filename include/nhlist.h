#pragma once

/*
类似于Linux链表，但是没有list_head
*/

/*
__nhlist_add
nhlist_add
nhlist_add_tail
__nhlist_del
nhlist_del
nhlist_replace
nhlist_entry
nhlist_do_each
nhlist_while_each
nhlist_do_each_prev
nhlist_while_each_prev
nhlist_do_each_entry
nhlist_while_each_entry
nhlist_do_each_entry_reverse
nhlist_while_each_entry_reverse
*/

#include <stddef.h>

struct list_nohead {
    struct list_nohead *next, *prev;
};

static inline void __nhlist_add(struct list_nohead *_new,
        struct list_nohead *prev,
        struct list_nohead *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

static inline void nhlist_add(struct list_nohead *_new, struct list_nohead *head)
{
    __nhlist_add(_new, head, head->next);
}

static inline void nhlist_add_tail(struct list_nohead *_new, struct list_nohead *head)
{
    __nhlist_add(_new, head->prev, head);
}

static inline void __nhlist_del(struct list_nohead * prev, struct list_nohead * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void nhlist_del(struct list_nohead *entry)
{
    __nhlist_del(entry->prev, entry->next);
}

static inline void nhlist_replace(struct list_nohead *old,
        struct list_nohead *_new)
{
    _new->next = old->next;
    _new->next->prev = _new;
    _new->prev = old->prev;
    _new->prev->next = _new;
}

#define nhlist_entry(ptr, type, member) \
    ((type *)((size_t)(ptr) - offsetof(type, member)))

#define nhlist_do_each(pos, head) \
    pos=(head); \
    do

#define nhlist_while_each(pos, head) \
    while ( pos=pos->next , pos != (head) );

#define nhlist_do_each_prev(pos, head) \
    pos=(head)->prev; \
    do

#define nhlist_while_each_prev(pos, head) \
    while ( pos != (head) && ( pos=pos->prev, true ) )

#define nhlist_do_each_entry(pos, head, member) \
    pos = list_entry((head), __typeof__(*pos), member); \
    do

#define nhlist_while_each_entry(pos, head, member) \
    while ( pos = list_entry((pos)->member.next, __typeof__(*(pos)), member), \
            &pos->member != (head) )

#define nhlist_do_each_entry_reverse(pos, head, member) \
    pos = list_entry((head)->prev, __typeof__(*pos), member); \
    do

#define nhlist_while_each_entry_reverse(pos, head, member) \
        while ( &pos->member != (head) && \
                (pos = list_entry((pos)->member.prev, __typeof__(*(pos)), member), true) )
