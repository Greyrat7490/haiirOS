#include "phys.h"
#include "io/io.h"
#include "memory/virt.h"
#include "paging.h"
#include "memory.h"

#define BOOT_SECTION_SIZE (8*1024*1024)   // reserve first 8MiB for bootloader and tmp frames

#define BITMAP_BLOCK_SIZE sizeof(uint8_t)

#define TMP_BITMAP_BASE 0x3f0000
#define TMP_BITMAP_SIZE (TMP_BITMAP_LAST_FRAME / FRAME_SIZE / BITMAP_BLOCK_SIZE - 1)
#define TMP_BITMAP_FIRST_FRAME 0x201000
#define TMP_BITMAP_LAST_FRAME TMP_BITMAP_BASE

static uint8_t* bitmap;
static uint64_t bitmap_size;
static uint64_t last_idx;
static uint64_t last_idx_bit = BITMAP_BLOCK_SIZE-1;

static void reserve_frames(uint64_t start_addr, uint64_t size) {
    if (size == 0) { return; }

    uint32_t pages = (size-1) / FRAME_SIZE + 1;

    uint32_t blocks = pages / BITMAP_BLOCK_SIZE;
    uint32_t start_block = start_addr / FRAME_SIZE / BITMAP_BLOCK_SIZE;
    uint32_t offset = start_addr / FRAME_SIZE % BITMAP_BLOCK_SIZE;

    uint32_t end_block = start_block + blocks;
    for (uint32_t i = start_block; i < end_block; i++) {
        bitmap[i] = 0xff >> offset;
    }

    if (blocks != 0) {
        bitmap[end_block] |= 0xff << (BITMAP_BLOCK_SIZE-offset);
    }

    uint8_t rest = pages % BITMAP_BLOCK_SIZE;
    if (rest != 0) {
        uint8_t bits = 0xff << (BITMAP_BLOCK_SIZE - rest);
        bitmap[end_block]   |= bits >> offset;
        bitmap[end_block+1] |= bits << (BITMAP_BLOCK_SIZE-offset);
    }
}

static void free_frames(uint64_t start_addr, uint64_t size) {
    if (size == 0) { return; }

    uint32_t pages = (size-1) / FRAME_SIZE + 1;

    uint32_t blocks = pages / BITMAP_BLOCK_SIZE;
    uint32_t start_block = start_addr / FRAME_SIZE / BITMAP_BLOCK_SIZE;
    uint32_t offset = start_addr / FRAME_SIZE % BITMAP_BLOCK_SIZE;

    uint32_t end_block = start_block + blocks;
    for (uint32_t i = start_block; i < end_block; i++) {
        bitmap[i] = ~(0xff >> offset);
    }

    if (blocks != 0) {
        bitmap[end_block] &= ~(0xff << (BITMAP_BLOCK_SIZE-offset));
    }

    uint8_t rest = pages % BITMAP_BLOCK_SIZE;
    if (rest != 0) {
        uint8_t bits = ~(0xff << (BITMAP_BLOCK_SIZE - rest));
        bitmap[end_block]   &= bits >> offset;
        bitmap[end_block+1] &= bits << (BITMAP_BLOCK_SIZE-offset);
    }
}

static void print_bitmap_block(uint8_t* last_val, uint64_t* size, uint64_t* base, uint64_t idx) {
    uint8_t block = bitmap[idx];

    // no rest in the block
    if (block == 0xff) {
        kprintln("free: %x %x", *base, *base + *size);
        *last_val = ~*last_val;
        *base += *size;
        *size = BITMAP_BLOCK_SIZE*FRAME_SIZE;
    } else if (block == 0) {
        kprintln("used: %x %x", *base, *base + *size);
        *last_val = ~*last_val;
        *base += *size;
        *size = BITMAP_BLOCK_SIZE*FRAME_SIZE;

    // some rest in the block
    } else {
        for (int32_t i = BITMAP_BLOCK_SIZE-1; i >= 0; i--) {
            uint8_t b = (block >> i) & 1;
            if (((*last_val) & 1) != b) {
                if (b == 1) {
                    kprintln("free: %x %x", *base, *base + *size);
                } else {
                    kprintln("used: %x %x", *base, *base + *size);
                }

                *last_val = ~*last_val;
                *base += *size;
                *size = FRAME_SIZE;
            } else {
                *size += FRAME_SIZE;
            }
        }
    }
}

static void create_tmp_bitmap(void) {
    bitmap = (uint8_t*)TMP_BITMAP_BASE;
    bitmap_size = TMP_BITMAP_SIZE;

    reserve_frames(0x0, TMP_BITMAP_FIRST_FRAME);
    free_frames(TMP_BITMAP_FIRST_FRAME, TMP_BITMAP_BASE-1);
}

static void map_bitmap(void) {
    uint64_t base = (uint64_t)bitmap;
    uint64_t size = bitmap_size;

    create_tmp_bitmap();

    for (uint32_t i = 0; i < size/PAGE_SIZE + 1; i++) {
        map_frame(to_page((uint64_t)base + i*PAGE_SIZE), to_frame((uint64_t)base + i*FRAME_SIZE), Present | Writeable);
    }

    bitmap = (uint8_t*)base;
    bitmap_size = size;
}

static uint64_t get_bitmap_size(memory_info_t* memory_info) {
    memory_map_t map = memory_info->map;

    uint64_t last_usable_addr = 0;
    for (uint16_t i = 0; i < map.count; i++) {
        if (map.entries[i].type == MEMORY_RANGE_USABLE) {
            last_usable_addr = map.entries[i].base + map.entries[i].len;
        }
    }

    return last_usable_addr / FRAME_SIZE / BITMAP_BLOCK_SIZE;
}

static void init_bitmap(memory_info_t* memory_info) {
    bitmap_size = get_bitmap_size(memory_info);
    bitmap = (uint8_t*)((uint64_t)memory_info->kernel_addr +
            ((memory_info->kernel_size + (FRAME_SIZE-1)) & ~(FRAME_SIZE-1))); // keep bitmap page aligned

    map_bitmap();

    // reserve all frames first
    for (uint64_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0xff;
    }
}

static void* pmm_alloc_unmapped_(uint64_t count) {
    if (count == 0) { return 0x0; }

    uint64_t continues_frames = 0;

    for (uint64_t i = last_idx; i < bitmap_size; i++) {
        uint8_t b = bitmap[i];
        if (b != 0xff) {
            for (int8_t j = last_idx_bit; j >= 0; j--) {
                if ((b >> j) == 0) {
                    continues_frames++;
                    if (continues_frames >= count) {
                        last_idx = i;
                        last_idx_bit = j;

                        void* addr = (void*)((i*BITMAP_BLOCK_SIZE + j - continues_frames) * FRAME_SIZE);
                        reserve_frames((uint64_t)addr, continues_frames * FRAME_SIZE);
                        return addr;
                    }
                } else {
                    continues_frames = 0;
                }
            }
        } else {
            continues_frames = 0;
        }

        last_idx_bit = BITMAP_BLOCK_SIZE-1;
    }

    return (void*)0x0;
}


void init_pmm(memory_info_t* memory_info) {
    init_bitmap(memory_info);

    for (uint16_t i = 0; i < memory_info->map.count; i++) {
        if (memory_info->map.entries[i].type == MEMORY_RANGE_USABLE) {
            free_frames(memory_info->map.entries[i].base, memory_info->map.entries[i].len);
        }
    }

    reserve_frames(0x0, BOOT_SECTION_SIZE);
    reserve_frames(memory_info->kernel_addr, memory_info->kernel_size);
    reserve_frames((uint64_t)bitmap, bitmap_size);
    reserve_frames(memory_info->vbe_framebuffer, memory_info->vbe_framebuffer_size);
}

bool pmm_is_free(uint64_t addr) {
    uint32_t block = addr / FRAME_SIZE / BITMAP_BLOCK_SIZE;
    uint32_t offset = addr / FRAME_SIZE % BITMAP_BLOCK_SIZE;
    return (bitmap[block] >> offset) == 0;
}

// return addr to count continues frames and reserves them in bitmap
void* pmm_alloc_unmapped(uint64_t count) {
    void* addr = pmm_alloc_unmapped_(count);

    // retry once
    if (addr == 0x0) {
        last_idx = 0;
        return pmm_alloc_unmapped_(count);
    }

    return addr;
}

void* pmm_alloc(uint64_t count) {
    void* addr = pmm_alloc_unmapped(count);
    map_frame(to_frame((uint64_t)addr), to_page((uint64_t)addr), Present | Writeable);
    return addr;
}

void pmm_free(page_t first_page, uint64_t count) {
    frame_t first_frame = (frame_t) to_phys(first_page);
    free_frames(first_frame, count*FRAME_SIZE);

    for (uint64_t i = 0; i < count; i++) {
        unmap_page(first_page + i*PAGE_SIZE);
    }
}

void pmm_free_unmapped(page_t first_page, uint64_t count) {
    frame_t first_frame = (frame_t) to_phys(first_page);
    free_frames(first_frame, count*FRAME_SIZE);
}

void print_frame_map(void) {
    uint8_t last_val = 0xff;

    uint64_t size = 0;
    uint64_t base = 0;
    for (uint64_t i = 0; i < bitmap_size; i++) {
        if (bitmap[i] != last_val) {
            print_bitmap_block(&last_val, &size, &base, i);
        } else {
            size += BITMAP_BLOCK_SIZE*FRAME_SIZE;
        }
    }

    if (last_val == 0) {
        kprintln("free: %x %x", base, base + size + FRAME_SIZE);
    } else {
        kprintln("used: %x %x", base, base + size + FRAME_SIZE);
    }
}
