#ifndef PHYS_H_
#define PHYS_H_

#include "types.h"
#include "boot/boot_info.h"
#include "memory/memory.h"

#define FRAME_SIZE 4096     // 4KiB

typedef uint64_t frame_t;

void init_pmm(memory_info_t* memory_info);

void* pmm_alloc_unmapped(uint64_t count);
void* pmm_alloc(uint64_t count);
void pmm_free(frame_t first_frame, uint64_t count);

void print_frame_map(void);

inline frame_t to_frame(uint64_t addr) { 
    return (frame_t) addr & ~(FRAME_SIZE-1);
}
uint64_t get_next_frame_addr(void);

#endif // PHYS_H_
