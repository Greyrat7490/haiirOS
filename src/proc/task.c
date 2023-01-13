#include "task.h"
#include "io/io.h"
#include "memory/Paging/hPaging.h"
#include "proc/scheduler.h"

struct tss {
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
typedef struct tss tss_t;

extern tss_t* tss_pointer;
extern void flush_tss(void);
extern uint64_t kernel_stack;
extern void enable_syscalls(void);

void init_tss(void) {
    tss_pointer->rsp0 = kernel_stack;
    flush_tss();
}

void init_syscalls(void) {
    enable_syscalls();
}

void add_task(const char* task_name, uint64_t func_addr) {
    uint64_t* user_pml4 = create_user_pml4();

    uint64_t user_stack_top = 0x1000000f000;
    for (int i = 0; i < 0xf; i++) {
        hFrame frame = alloc_frame();
        hPage page = get_hPage(user_stack_top - i * 0x1000);
        map_user_frame(user_pml4, page, frame, Present | Writeable | User);
    }

    uint64_t user_func_addr = 0x10000010000 + (func_addr & 0xfff);
    map_user_frame(user_pml4, get_hPage(user_func_addr), get_hFrame(func_addr), Present | Writeable | User);
    map_user_frame(user_pml4, get_hPage(user_func_addr + PAGE_SIZE), get_hFrame(func_addr + PAGE_SIZE), Present | Writeable | User);

    // map .rodata/.data (after .text)    (.text ~65KiB (release) / .text ~57KiB (debug))
    // only tmp until executable instead of function
#ifdef DEBUG
    uint64_t text_seg_size = 0x3900;
#else
    uint64_t text_seg_size = 0x5900;
#endif

    uint64_t data_seg_phys = (func_addr & ~0xfff) + text_seg_size;
    uint64_t data_seg_virt = 0x10000010000 + text_seg_size;                                 // both data and rodata writeable (no separation)
    map_user_frame(user_pml4, get_hPage(data_seg_virt), get_hFrame(data_seg_phys), Present | Writeable | User);
    map_user_frame(user_pml4, get_hPage(data_seg_virt + PAGE_SIZE), get_hFrame(data_seg_phys + PAGE_SIZE), Present | Writeable | User);

    add_tcb((TCB_t) {
        .name = task_name,
        .user_stack = user_stack_top,
        .user_func = user_func_addr,
        .virt_addr_space = (uint64_t) user_pml4
    });
}
