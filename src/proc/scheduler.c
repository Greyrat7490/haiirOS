#include "scheduler.h"
#include "proc/task.h"
#include "io/io.h"

extern void jump_usermode(uint64_t usr_stack_top, uint64_t usr_func_addr, uint64_t pml4_addr);

#define MAX_TASKS 10

static TCB_t s_tasks[MAX_TASKS];

static uint64_t s_cur_task = 0;
static uint64_t s_tasks_count = 0;

void add_tcb(TCB_t task) {
    if (s_tasks_count >= MAX_TASKS) {
        kprintln("only 10 tasks max are allowed yet");
        return;
    }

    s_tasks[s_tasks_count] = task;
    s_tasks_count++;
}

void start_scheduler(void) {
    kprintln("start scheduler...");

    kprintln("func_addr:  %x", s_tasks[0].user_func);
    kprintln("pml4_addr:  %x", s_tasks[0].virt_addr_space);
    kprintln("stack_addr: %x", s_tasks[0].user_stack);

    if (s_tasks[0].user_func == 0x0 || s_tasks[0].user_stack == 0x0 || s_tasks[0].virt_addr_space == 0x0) {
        kprintln("Error no init task set");
        return;
    }

    jump_usermode(s_tasks[0].user_stack, s_tasks[0].user_func, s_tasks[0].virt_addr_space);
}

void switch_task(void) {
    kprintln("next task...");

    s_cur_task++;
    if (s_cur_task >= MAX_TASKS || s_tasks[s_cur_task].user_func == 0x0) {
        kprintln("no tasks left...");
        while(1) __asm__("hlt");
    }

    jump_usermode(s_tasks[s_cur_task].user_stack, s_tasks[s_cur_task].user_func, s_tasks[s_cur_task].virt_addr_space);
}
