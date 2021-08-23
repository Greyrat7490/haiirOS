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
void show_entries( int ptEntries, int ptTables );

uint16_t get_lv4_index( const uint64_t virt_addr );
uint16_t get_lv3_index( const uint64_t virt_addr );
uint16_t get_lv2_index( const uint64_t virt_addr );
uint16_t get_lv1_index( const uint64_t virt_addr );

uint64_t to_phys( const uint64_t virt_addr );

void map_to( const hPage page, const hFrame frame, const PageFlags flags );

bool is_addr_present( const uint64_t virt_addr );

void test_mapping();

#endif
