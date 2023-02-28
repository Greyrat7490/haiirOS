#include "task.h"
#include "io/io.h"
#include "memory/paging.h"
#include "proc/scheduler.h"

extern uint64_t kernel_stack;
extern void enable_syscalls(void);

void init_syscalls(void) {
    enable_syscalls();
}

void add_task(const char* task_name, uint64_t func_addr) {
    uint64_t* user_pml4 = create_user_pml4();

    uint64_t user_stack_top = 0x1000000f000;
    uint32_t pagesCount = 0xf;

    uint64_t phys_addr_start = (uint64_t)pmm_alloc_unmapped(pagesCount) + pagesCount*PAGE_SIZE;
    for (uint32_t i = 0; i < pagesCount ; i++) {
        frame_t frame = to_frame(phys_addr_start - i*PAGE_SIZE);
        page_t page = to_page(user_stack_top - i*PAGE_SIZE);
        map_user_frame(user_pml4, page, frame, Present | Writeable | User);
    }

    uint64_t user_func_addr = 0x10000010000 + (func_addr & 0xfff);
    map_user_frame(user_pml4, to_page(user_func_addr), to_frame(func_addr), Present | Writeable | User);
    map_user_frame(user_pml4, to_page(user_func_addr + PAGE_SIZE), to_frame(func_addr + PAGE_SIZE), Present | Writeable | User);

    // map .rodata/.data (after .text)    (.text ~65KiB (release) / .text ~57KiB (debug))
    // only tmp until executable instead of function
#ifdef DEBUG
    uint64_t text_seg_size = 0x3900;
#else
    uint64_t text_seg_size = 0x5900;
#endif

    uint64_t data_seg_phys = (func_addr & ~0xfff) + text_seg_size;
    uint64_t data_seg_virt = 0x10000010000 + text_seg_size;                                 // both data and rodata writeable (no separation)
    map_user_frame(user_pml4, to_page(data_seg_virt), to_frame(data_seg_phys), Present | Writeable | User);
    map_user_frame(user_pml4, to_page(data_seg_virt + PAGE_SIZE), to_frame(data_seg_phys + PAGE_SIZE), Present | Writeable | User);

    add_tcb((TCB_t) {
        .name = task_name,
        .user_stack = user_stack_top,
        .user_func = user_func_addr,
        .virt_addr_space = (uint64_t) user_pml4
    });
}
