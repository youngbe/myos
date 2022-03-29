#pragma once

typedef unsigned long int size_t;
typedef signed long int ssize_t;

#ifdef __GNUG__
#define NULL __null
#else   /* G++ */
#ifndef __cplusplus
#define NULL ((void *)0)
#else   /* C++ */
#define NULL 0
#endif  /* C++ */
#endif  /* G++ */

//#define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#define offsetof(type, member) ((size_t)&((type*)0)->member)
