#ifndef PAGING_H_
#define PAGING_H_

#include "types.h"
#include "virt.h"
#include "phys.h"

typedef uint64_t PageFlags;
#define Present         1
#define Writeable       (1 << 1)
#define User            (1 << 2)
#define WriteThrough    (1 << 3)
#define DisableCache    (1 << 4)
#define Accessed        (1 << 5)
#define Dirty           (1 << 6)
#define Huge            (1 << 7)
#define Global          (1 << 8)
#define ExecDisable     (1ull << 63)

typedef enum {
    PROT_READ   = 0x1,
    PROT_WRITE  = 0x2,
    PROT_EXEC   = 0x4,
    PROT_NONE   = 0x0   // Page can not be accessed
} ProtFlags;


void init_paging(void);

void map_frame(page_t page, frame_t frame, PageFlags flags);
void map_user_frame(uint64_t* pml4_table, page_t page, frame_t frame, PageFlags flags);
void unmap_page(page_t page);
void unmap_user_page(uint64_t* pml4_table, page_t page);
uint64_t* create_user_pml4(void);

uint64_t to_phys(uint64_t virt_addr);
uint64_t to_usr_phys(uint64_t* pml4_table, uint64_t virt_addr);

bool is_addr_present(uint64_t virt_addr);
bool is_usr_addr_present(uint64_t* pml4_table, uint64_t virt_addr);

void test_mapping(void);
void show_entries(uint16_t ptEntries, uint16_t ptTables);

#endif // PAGING_H_
