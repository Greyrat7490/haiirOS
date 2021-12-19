#ifndef H_TASK
#define H_TASK

#include "types.h"
#include "scheduler.h"

void init_tss();

void add_task(const char* task_name, uint64_t func_addr);

#endif // H_TASK

