#pragma once
#include "public.h"
#include <stdint.h>

extern size_t const cores_num;

extern struct TSS64 *const tsss;

// 页表
extern uint64_t kernel_pt1s[64][512];
extern const uint64_t halt_pt0[512];
extern uint64_t (*const page_tables)[512];
extern uint_fast16_t *const pte_nums;
extern uint64_t (**const free_page_tables)[512];
extern size_t free_page_tables_num;

extern uint64_t *const free_pages;
extern size_t free_pages_num;

extern struct __attribute__((aligned(16))){uint8_t padding[HALT_STACK_SIZE];}
*const halt_stacks;
extern Thread **const running_threads;
