#include "hFrame.h"
#include "io/io.h"
#include "memory/Paging/Page/hPage.h"
#include "memory/Paging/hPaging.h"
#include "memory/memory.h"

static uint64_t NEXT_FRAME = 0x300000;

#define BOOT_SECTION_SIZE (8*1024*1024)   // reserve first 8MiB for bootloader and tmp frames

#define BITMAP_BLOCK_SIZE sizeof(uint8_t)

static uint8_t* bitmap;
static uint64_t bitmap_size;

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
        bitmap[end_block] |= bits >> offset;
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

static void init_bitmap(memory_info_t* memory_info) {
    memory_map_t map = memory_info->map;

    uint64_t last_usable_addr = 0;
    for (uint16_t i = 0; i < map.count; i++) {
        if (map.entries[i].type == MEMORY_RANGE_USABLE) {
            last_usable_addr = map.entries[i].base + map.entries[i].len;
        }
    }
    bitmap_size = last_usable_addr / FRAME_SIZE / BITMAP_BLOCK_SIZE;

    bitmap = (uint8_t*)((uint64_t)memory_info->kernel_addr +
            ((memory_info->kernel_size + (FRAME_SIZE-1)) & ~(FRAME_SIZE-1))); // keep bitmap page aligned

    for (uint32_t i = 0; i < bitmap_size/PAGE_SIZE + 1; i++) {
        map_frame(get_hPage((uint64_t)bitmap + i*PAGE_SIZE), get_hFrame((uint64_t)bitmap + i*FRAME_SIZE), Present | Writeable);
    }

    // reserve all frames first
    for (uint64_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0xff;
    }
}

void init_frame_allocator(memory_info_t* memory_info) {
    init_bitmap(memory_info);

    for (uint16_t i = 0; i < memory_info->map.count; i++) {
        if (memory_info->map.entries[i].type == MEMORY_RANGE_USABLE) {
            free_frames(memory_info->map.entries[i].base, memory_info->map.entries[i].len);
        }
    }

    reserve_frames(0x0, BOOT_SECTION_SIZE);
    reserve_frames(memory_info->kernel_addr, memory_info->kernel_size);
    reserve_frames(memory_info->vbe_framebuffer, memory_info->vbe_framebuffer_size);
    reserve_frames((uint64_t)bitmap, bitmap_size);
}

hFrame get_hFrame(uint64_t containing_addr) {
    return (hFrame) { containing_addr - (containing_addr % FRAME_SIZE) };
}

uint64_t get_next_frame_addr(void) {
    return NEXT_FRAME;
}

// TODO: proper allocator in work
hFrame alloc_frame(void) {
    hFrame new_frame = { NEXT_FRAME };

    NEXT_FRAME += FRAME_SIZE;

    return new_frame;
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
