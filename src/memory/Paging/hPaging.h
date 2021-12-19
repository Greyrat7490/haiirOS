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

void map_frame(hPage page, hFrame frame, PageFlags flags);
void map_user_frame(uint64_t* pml4_table, hPage page, hFrame frame, PageFlags flags);
uint64_t* create_user_pml4();

uint64_t to_phys(uint64_t virt_addr);
bool is_addr_present(uint64_t virt_addr);

void test_mapping();
void show_entries(uint16_t ptEntries, uint16_t ptTables);

#endif
