#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct List             List;
typedef struct Dqlist           Dqlist;

struct List {
    List *next;
};

struct Dqlist {
    Dqlist *next;
    Dqlist *prev;
};

#define list_entry(ptr, type, member) \
    ((type *)( (uint8_t *)(ptr) - offsetof(type,member) ))

#define list_for_each(iterator, index, code) \
    do \
{ \
    __auto_type const temp_i=(index); \
    __auto_type iterator=temp_i; \
    do \
    { \
        {code} \
        iterator=iterator->next; \
    } \
    while ( iterator != temp_i ); \
}while (0)

#define list_for_each_safe(iterator, index, code) \
    do \
{ \
    __auto_type const temp_i=(index); \
    if ( temp_i != NULL ) \
    { \
        __auto_type iterator=temp_i; \
        do \
        { \
            {code} \
            iterator=iterator->next; \
        } \
        while ( iterator != temp_i ); \
    } \
}while (0)

#define list_for_each_entry(iterator, index, type, member, code) \
    do \
{ \
    __auto_type const temp_i=(index); \
    __auto_type iter=temp_i; \
    do \
    { \
        type* iterator=list_entry(iter, type, member); \
        {code} \
        iter=iter->next; \
    } \
    while ( iter != temp_i ); \
}while(0)

#define list_for_each_entry_safe(iterator, index, type, member, code) \
    do \
{ \
    __auto_type const temp_i=(index); \
    if ( temp_i != NULL ) \
    { \
        __auto_type iter=temp_i; \
        do \
        { \
            type* iterator=list_entry(iter, type, member); \
            {code} \
            iter=iter->next; \
        } \
        while ( iter != temp_i ); \
    } \
}while(0)

static inline void list_add(List *new, List *prev)
{
    List*const next=prev->next;
    new->next=next;
    prev->next=new;
}

static inline void list_add_safe(List *new, List **pprev)
{
    List *const prev=*pprev;
    if ( prev == NULL )
    {
        *pprev=new->next=new;
    }
    else
    {
        List*const next=prev->next;
        new->next=next;
        prev->next=new;
    }
}

static inline void dqlist_add(Dqlist *new, Dqlist *prev)
{
    Dqlist*const next=prev->next;
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next= new;
}

static inline void dq_list_add_safe(Dqlist *new, Dqlist **pprev)
{
    Dqlist *const prev=*pprev;
    if ( prev == NULL )
    {
        *pprev=new->prev=new->next=new;
    }
    else
    {
        Dqlist*const next=prev->next;
        next->prev = new;
        new->next = next;
        new->prev = prev;
        prev->next= new;
    }
}
