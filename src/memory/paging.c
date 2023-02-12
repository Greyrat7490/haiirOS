#include "paging.h"

#include "types.h"
#include "io/io.h"
#include "phys.h"
#include "virt.h"


typedef struct
{
    uint64_t entries[512];
} PTTable;

typedef struct
{
    uint64_t pt_tables[512];
} PDTable;

typedef struct
{
    uint64_t pd_tables[512];
} PDPTable;

typedef struct
{
    uint64_t pdp_tables[512];
} PML4Table;


static PML4Table* s_pml4_table;

// TODO: add mmap to check if memory area is available

// bits 11 - 9 are free to use in page tables and directories
enum PageDirType {
    PML4      = (3 << 9 ),   // _11
    PDP       = (1 << 10),   // _10
    PD        = (1 << 9 ),   // _01
};
enum PageDirError {
    NOT_PRESENT     = (1 << 11),   // 1__   // cannot reach paging table, because a directory is not present
    MISSING_TABLE   = (0 << 11),   // 0__   // bit 9 or 10 is set (which PageDirType)
    ANY_ERROR       = (7 << 9 ),   // 111
};

#define TMP_ADDR 0x200000

// TODO: check if address is derefrenceable
// without causing page fault (important in get_entry_by_index)

static bool is_entry_valid(uint64_t* entry) {
    return !(*entry & ANY_ERROR);
}

static bool is_entry_present(uint64_t* entry) {
    return *entry & Present;
}

static uint16_t get_lv4_index(uint64_t virt_addr) {
    return (uint16_t) ((virt_addr >> 39) & 0x1ff);
}

static uint16_t get_lv3_index(uint64_t virt_addr) {
    return (uint16_t) ((virt_addr >> 30) & 0x1ff);
}

static uint16_t get_lv2_index(uint64_t virt_addr) {
    return (uint16_t) ((virt_addr >> 21) & 0x1ff);
}

static uint16_t get_lv1_index(uint64_t virt_addr) {
    return (uint16_t) ((virt_addr >> 12) & 0x1ff);
}

static void flush_TLB(void* m) {
    // invlpg uses eax register (32bit)
    // = 2x 16bit fields
    // -> sets "random" values for the first two fields of the console (addr of m)

    // fixed by not using eax register
    __asm__ volatile ("invlpg (%0)" : : "r"(m) : "memory");
}

static uint64_t* get_temp_entry(void) {
    const uint64_t virt_addr = TMP_ADDR;

    const uint16_t i1 = get_lv1_index(virt_addr);
    const uint16_t i2 = get_lv2_index(virt_addr);
    const uint16_t i3 = get_lv3_index(virt_addr);
    const uint16_t i4 = get_lv4_index(virt_addr);

    uint64_t* pml4_entry = &s_pml4_table->pdp_tables[i4];

    PDPTable* pdp_table = (PDPTable*) (*pml4_entry & 0xfffff000);
    uint64_t* pdp_entry = &pdp_table->pd_tables[i3];

    PDTable* pd_table = (PDTable*) (*pdp_entry & 0xfffff000);
    uint64_t* pd_entry = &pd_table->pt_tables[i2];

    PTTable* pt_table = (PTTable*) (*pd_entry & 0xfffff000);
    return &pt_table->entries[i1];
}

static uint64_t* tmp_map(uint64_t phys_addr) {
    uint64_t* tmp_entry = get_temp_entry();

    frame_t frame = to_frame(phys_addr);
    *tmp_entry = frame | Present | Writeable;

    flush_TLB((void*) TMP_ADDR);
    return (uint64_t*) (TMP_ADDR + (phys_addr & 0xfff));
}

// if pointer is not derefrenceable it will
// return a pointer with a temporary mapped virtual address
static void* valid_pointer(void* pointer) {
    // temporary way to check if an address is derefrenceable
    // without causing a page fault if not
    if ((uint64_t) pointer >= 0x800000)
        return tmp_map((uint64_t) pointer);

    return pointer;
}

static bool get_table_from_entry(uint64_t* entry, enum PageDirType type, uint64_t** ppTable) {
    entry = (uint64_t*) valid_pointer(entry);

    *ppTable = (uint64_t*) (*entry & 0xfffff000);
    if (!is_entry_present(entry)) {

        // physical addr 0x0 is not allowed -> not a table
        // or frame is not allocated
        if ((uint64_t) *ppTable == 0 || (uint64_t) *ppTable >= get_next_frame_addr())
            *entry |= MISSING_TABLE | type;
        else // table exist but is not present
            *entry |= NOT_PRESENT | type;

        return false;
    }

    return true;
}

static uint64_t* get_entry_by_index(PML4Table* pml4_table, uint16_t pml4_index, uint16_t pdp_index, uint16_t pd_index, uint16_t pt_index) {
    uint64_t* pml4_entry = &pml4_table->pdp_tables[pml4_index];
    PDPTable* pdp_table = 0x0;
    if (!get_table_from_entry(pml4_entry, PML4, (uint64_t**) &pdp_table))
        return (uint64_t*) pml4_entry;

    uint64_t* pdp_entry = &pdp_table->pd_tables[pdp_index];
    PDTable* pd_table = 0x0;
    if (!get_table_from_entry(pdp_entry, PDP, (uint64_t**) &pd_table))
        return (uint64_t*) valid_pointer(pdp_entry);

    uint64_t* pd_entry = &pd_table->pt_tables[pd_index];
    PTTable* pt_table = 0x0;
    if (!get_table_from_entry(pd_entry, PD, (uint64_t**) &pt_table))
        return (uint64_t*) valid_pointer(pd_entry);

    pt_table = (PTTable*) valid_pointer(pt_table);
    return &pt_table->entries[pt_index];
}

static uint64_t* get_entry(PML4Table* pml4_table, uint64_t virt_addr) {
    uint16_t i1 = get_lv1_index(virt_addr);
    uint16_t i2 = get_lv2_index(virt_addr);
    uint16_t i3 = get_lv3_index(virt_addr);
    uint16_t i4 = get_lv4_index(virt_addr);

    return get_entry_by_index(pml4_table, i4, i3, i2, i1);
}

// return virtual address to the table
static uint64_t* create_table(uint64_t* entry, PageFlags flags) {
    void* table = pmm_alloc_unmapped(1);
    *entry = (uint64_t)table | flags;
    return tmp_map((uint64_t)table);
}

static void create_missing_tables(uint64_t* entry, page_t page, frame_t frame, PageFlags flags) {
    if ((*entry & PML4) == PML4) {
        PDPTable* pdp_table = (PDPTable*) create_table(entry, flags);
        kprintln("%s", "pdp table created");

        uint64_t i3 = get_lv3_index(page);
        entry = &pdp_table->pd_tables[i3];
        *entry = PDP;
    }

    if (*entry & PDP) {
        PDTable* pd_table = (PDTable*) create_table(entry, flags);
        kprintln("%s", "pd table created");

        uint64_t i2 = get_lv2_index(page);
        entry = &pd_table->pt_tables[i2];
        *entry = PD;
    }

    if (*entry & PD) {
        PTTable* pt_table = (PTTable*) create_table(entry, flags);
        kprintln("%s", "pt table created");

        uint64_t i1 = get_lv1_index(page);
        pt_table->entries[i1] = frame | flags;
    }
}


void init_paging(void) {
    uint64_t addr;

     __asm__ (
        "mov %%cr3, %%rax\n" // cr3 -> rax
        "mov %%rax, %0\n"    // rax -> addr
        : "=m" (addr)
        : // no input
        : "%rax"
    );

    kprintln("pml4 addr: %x", addr);

    s_pml4_table = (PML4Table*) addr;
}

// 0x0 (nullptr) if no valid and/or present entry exists
uint64_t to_phys(uint64_t virt_addr) {
    uint64_t* entry = get_entry(s_pml4_table, virt_addr);

    if (!is_entry_valid(entry)) {
        kprintln("virt_addr (%x) is not valid (has no entry)", virt_addr);
        return 0x0;
    }

    if (!is_entry_present(entry)) {
        kprintln("virt_addr(%x) is not present", virt_addr);
        return 0x0;
    }

    return (*entry & ~((uint64_t) 0xfff)) + (virt_addr & 0xfff);
}

uint64_t to_usr_phys(uint64_t* pml4_table, uint64_t virt_addr) {
    uint64_t* entry = get_entry((PML4Table*) pml4_table, virt_addr);

    if (!is_entry_valid(entry)) {
        kprintln("virt_addr (%x) is not valid (has no entry)", virt_addr);
        return 0x0;
    }

    if (!is_entry_present(entry)) {
        kprintln("virt_addr(%x) is not present", virt_addr);
        return 0x0;
    }

    return (*entry & ~((uint64_t) 0xfff)) + (virt_addr & 0xfff);
}

static void map(uint64_t* pml4_table, page_t page, frame_t frame, PageFlags flags) {
    uint64_t* entry = get_entry((PML4Table*) pml4_table, page);

    if (is_entry_valid(entry)) {
        *entry = frame | flags;

        flush_TLB((void*) page);
    } else {
        if (!(*entry & NOT_PRESENT)) { // MISSING_TABLE
            create_missing_tables(entry, page, frame, flags);
            flush_TLB((void*) page);
            return;
        }

        // TODO set present of parent table entries 
        kprintln("A Page directory is not present -> could not get pt entry");
    }
}

static void unmap(uint64_t* pml4_table, page_t page) {
    uint64_t* entry = get_entry((PML4Table*) pml4_table, page);

    if (is_entry_valid(entry)) {
        *entry = 0x0;
        flush_TLB((void*) page);
    }
}


void map_frame(page_t page, frame_t frame, PageFlags flags) {
    map((uint64_t*) s_pml4_table, page, frame, flags);
}

void map_user_frame(uint64_t* pml4_table, page_t page, frame_t frame, PageFlags flags) {
    map(pml4_table, page, frame, flags);
}

void unmap_page(page_t page) {
    unmap((uint64_t*) s_pml4_table, page);
}

void unmap_user_page(uint64_t* pml4_table, page_t page) {
    unmap(pml4_table, page);
}

uint64_t* create_user_pml4(void) {
    PML4Table* pml4_addr = (PML4Table*)pmm_alloc_unmapped(1);
    map_frame(to_page((uint64_t) pml4_addr), to_frame((uint64_t) pml4_addr), Present | Writeable | User);

    for (uint16_t i = 1; i < 512; i++)
        pml4_addr->pdp_tables[i] = 0x0;

    // for interrupts, exceptions, syscalls and rest of the jump_usermode function (and more)
    pml4_addr->pdp_tables[0] = s_pml4_table->pdp_tables[0];

    return (uint64_t*) pml4_addr;
}

bool is_addr_present(uint64_t virt_addr) {
    uint64_t* entry = get_entry(s_pml4_table, virt_addr);

    if (is_entry_valid(entry))
        return is_entry_present(entry);

    return false;
}

bool is_usr_addr_present(uint64_t* pml4_table, uint64_t virt_addr) {
    uint64_t* entry = get_entry((PML4Table*) pml4_table, virt_addr);

    if (is_entry_valid(entry))
        return is_entry_present(entry);

    return false;
}

void show_entries(uint16_t ptEntries, uint16_t ptTables) {
    kprintln("pml4_table addr %x", s_pml4_table);

    for (uint16_t i2 = 0; i2 < ptTables; i2++) {
        uint64_t* entry = get_entry_by_index(s_pml4_table, 0, 0, i2, 0);
        kprintln("pd_table[%d] %x", i2, *entry);

        for (uint16_t i1 = 0; i1 < ptEntries; i1++) {
            uint64_t* entry = get_entry_by_index(s_pml4_table, 0, 0, i2, i1);
            kprintln("  pt_table[%d] entry[%d] %x", i2, i1, *entry);
        }
    }
}

// virt         -> phys
// 0x2000       -> 0xb8000
// 0x800000     -> 0x3000
// 0xfffffff000 -> 0xfff000
void test_mapping(void) {
    kclear_screen();
    // fixed
    // println("**<- flush_TLB clears the first two fields (first 32bit) of the console");

    // test how much is mapped (should be 8MiB)
    kprintln("%x is present = %b", 0x7fffff, is_addr_present(0x7fffff));
    kprintln("%x is present = %b", 0x800000, is_addr_present(0x800000));
    kprintln("%x is present = %b", 0xfffffff000, is_addr_present(0xfffffff000));


    frame_t frame1 = to_frame(0xb8000);
    frame_t frame2 = to_frame(0x3000);
    frame_t frame3 = to_frame(0xffffff);

    page_t page1 = to_page(0x2000);
    page_t page2 = to_page(0x800000);
    page_t page3 = to_page(0xfffffff000);

    uint16_t* testAddr1 = (uint16_t*) 0x2000;
    uint16_t* testAddr2 = (uint16_t*) 0x800010;
    uint64_t* testAddr3 = (uint64_t*) 0xfffffffff0;

    kprintln("before:");
    kprintln("testAddr1 virt %x -> phys %x", (uint64_t) testAddr1, to_phys((uint64_t) testAddr1));
    kprintln("testAddr2 virt %x -> phys %x", (uint64_t) testAddr2, to_phys((uint64_t) testAddr2));
    kprintln("testAddr3 virt %x -> phys %x", (uint64_t) testAddr3, to_phys((uint64_t) testAddr3));
    kprintln("testAddr1 val: %d ", *testAddr1);
    kprintln("*testAddr2 would cause a page fault");

    map_frame(page1, frame1, Present | Writeable);
    map_frame(page2, frame2, Present | Writeable); // needs to allocate a new frame for a new PageTable
    map_frame(page3, frame3, Present | Writeable); // needs to allocated several new frames for new PageTables

    kprintln("after:");
    kprintln("testAddr1 virt %x -> phys %x", (uint64_t) testAddr1, to_phys((uint64_t) testAddr1));
    kprintln("testAddr2 virt %x -> phys %x", (uint64_t) testAddr2, to_phys((uint64_t) testAddr2));
    kprintln("testAddr3 virt %x -> phys %x", (uint64_t) testAddr3, to_phys((uint64_t) testAddr3));

    for (uint16_t i = 24 * 80; i < 25 * 80; i++)
        *(testAddr1 + i) = WHITE << 12;

    *testAddr3 = 86;

    kprintln("after '*testAddr3 = 86':");
    kprintln("testAddr3 val: %d", *testAddr3);


    kprintln("%x is present = %b", 0x800000, is_addr_present(0x800000));
    kprintln("%x is present = %b", 0xfffffff000, is_addr_present(0xfffffff000));
}
