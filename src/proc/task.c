#include "task.h"
#include "types.h"

extern tss_t* tss_pointer;
extern void flush_tss();
extern uint64_t kernel_stack;

void init_tss() {
    set_kernel_stack(kernel_stack);
    flush_tss();
}

void set_kernel_stack(uint64_t stack) {
    tss_pointer->rsp0 = stack;
}
