#ifndef H_TASK
#define H_TASK

#include "types.h"

typedef struct TaskControlBlock {
    const char* name;
    void* kernel_stack;
    int64_t virt_addr_space;
    struct TaskControlBlock* next_task;
    int state;
} TCB_t;

struct tss_s {
    uint32_t reserved0;
    
    uint64_t rsp0; // kernel stack pointer (ring 0)
    uint64_t rsp1; // ring 1 stack pointer
    uint64_t rsp2; // ring 2 stack pointer
    
    uint64_t reserved1;

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_addr;
} __attribute__((packed));

typedef struct tss_s tss_t;

void init_tss();

void set_kernel_stack(uint64_t stack);

#endif // H_TASK

