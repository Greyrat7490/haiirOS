#ifndef FRAME_H_
#define FRAME_H_

#include "types.h"
#include "boot/boot_info.h"
#include "memory/memory.h"

#define FRAME_SIZE 4096     // 4KiB

typedef struct
{
    uint64_t start_addr;
} hFrame;

void init_frame_allocator(memory_info_t* memory_info);

hFrame get_hFrame(uint64_t containing_addr);
hFrame alloc_frame(void);
uint64_t get_next_frame_addr(void);
// TODO
void* pmm_alloc(uint64_t size);
void* pmm_free(uint64_t size);

void print_frame_map(void);

#endif // FRAME_H_
