#ifndef H_PAGING
#define H_PAGING

#include "types.h"
#include "Page/hPage.h"
#include "Frame/hFrame.h"

typedef enum {
    Present = 1,
    Writeable = 1 << 1,
    User = 1 << 2,
    WriteThrough = 1 << 3,
    DisableCache = 1 << 4,
    Accessed = 1 << 5,
    Dirty = 1 << 6,
    Huge = 1 << 7,
    Global = 1 << 8
} PageFlags;

void init_paging();
void show_entries( uint16_t ptEntries, uint16_t ptTables );

uint64_t to_phys( uint64_t virt_addr );

void map_to( hPage page, hFrame frame, PageFlags flags );

bool is_addr_present( uint64_t virt_addr );

void test_mapping();

#endif
