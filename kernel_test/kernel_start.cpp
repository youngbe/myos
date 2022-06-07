#include "public.h"

struct list_nohead *index_sched_threads;
tsl_mutex sched_threads_mutex;

extern "C" __attribute__((noreturn)) void kernel_start()
{
    kernel_abort("kernel started!");
}
