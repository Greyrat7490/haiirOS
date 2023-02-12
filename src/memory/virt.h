#ifndef VIRT_H_
#define VIRT_H_

#include "types.h"

#define PAGE_SIZE 4096  // 4KiB

typedef uint64_t page_t;

inline page_t to_page(uint64_t addr) { 
    return (page_t) (addr + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}

#endif // VIRT_H_
