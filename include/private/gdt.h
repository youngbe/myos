#pragma once

#define GDT_ENTRY_NULL      0
#define GDT_ENTRY_CS        1
#define GDT_ENTRY_CS_USER   2
#define GDT_ENTRY_DS_USER   3
#define GDT_ENTRY_TSS       4

#define __CS        (GDT_ENTRY_CS<<3)
#define __CS_USER   ((GDT_ENTRY_CS_USER<<3)|0b11)
#define __DS        (GDT_ENTRY_NULL<<3)
#define __DS_USER   ((GDT_ENTRY_DS_USER<<3)|0b11)
#define __TSS       (GDT_ENTRY_TSS<<3)
