#include "hPaging.h"

#include "types.h"
#include "io/io.h"

#include "memory/Paging/Frame/hFrame.h"
#include "memory/Paging/Page/hPage.h"


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


static PML4Table* pml4_table;

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

static uint64_t* get_temp_entry() {
    const uint64_t virt_addr = 0x400000;

    const uint16_t i1 = get_lv1_index(virt_addr);
    const uint16_t i2 = get_lv2_index(virt_addr);
    const uint16_t i3 = get_lv3_index(virt_addr);
    const uint16_t i4 = get_lv4_index(virt_addr);

    uint64_t* pml4_entry = &pml4_table->pdp_tables[i4];

    PDPTable* pdp_table = (PDPTable*) (*pml4_entry & 0xfffff000);
    uint64_t* pdp_entry = &pdp_table->pd_tables[i3];

    PDTable* pd_table = (PDTable*) (*pdp_entry & 0xfffff000);
    uint64_t* pd_entry = &pd_table->pt_tables[i2];

    PTTable* pt_table = (PTTable*) (*pd_entry & 0xfffff000);
    return &pt_table->entries[i1];
}

static uint64_t* tmp_map(uint64_t phys_addr) {
    uint64_t* tmp_entry = get_temp_entry();

    hFrame frame = get_hFrame(phys_addr);
    *tmp_entry = frame.start_addr | Present | Writeable;

    flush_TLB((void*) 0x400000);
    return (uint64_t*) (0x400000 + (phys_addr & 0xfff));
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

static uint64_t* get_entry_by_index(uint16_t pml4_index, uint16_t pdp_index, uint16_t pd_index, uint16_t pt_index) {
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

static uint64_t* get_entry(uint64_t virt_addr) {
    uint16_t i1 = get_lv1_index(virt_addr);
    uint16_t i2 = get_lv2_index(virt_addr);
    uint16_t i3 = get_lv3_index(virt_addr);
    uint16_t i4 = get_lv4_index(virt_addr);

    return get_entry_by_index(i4, i3, i2, i1);
}

// return virtual address to the table
static uint64_t* create_table(uint64_t* entry, PageFlags flags) {
    hFrame table_frame = alloc_frame();
    *entry = table_frame.start_addr | flags;

    return tmp_map(table_frame.start_addr);
}

// TODO: support creating tables in unmapped frames
static void create_missing_tables(uint64_t* entry, hPage page, hFrame frame, PageFlags flags) {
    if ((*entry & PML4) == PML4) {
        PDPTable* pdp_table = (PDPTable*) create_table(entry, flags);
        println("%s", "pdp table created");

        uint64_t i3 = get_lv3_index(page.start_addr);
        entry = &pdp_table->pd_tables[i3];
        *entry = PDP;
    }

    if (*entry & PDP) {
        PDTable* pd_table = (PDTable*) create_table(entry, flags);
        println("%s", "pd table created");

        uint64_t i2 = get_lv2_index(page.start_addr);
        entry = &pd_table->pt_tables[i2];
        *entry = PD;
    }

    if (*entry & PD) {
        PTTable* pt_table = (PTTable*) create_table(entry, flags);
        println("%s", "pt table created");

        uint64_t i1 = get_lv1_index(page.start_addr);
        pt_table->entries[i1] = frame.start_addr | flags;
    }
}


void init_paging() {
    uint64_t addr;

     __asm__ (
        "mov %%cr3, %%rax\n" // cr3 -> rax
        "mov %%rax, %0\n"    // rax -> addr
        : "=m" (addr)
        : // no input
        : "%rax"
    );

    println("pml4 addr: %x", addr);

    pml4_table = (PML4Table*) addr;
}

// 0x0 (nullptr) if no valid and/or present entry exists
uint64_t to_phys(uint64_t virt_addr) {
    uint64_t* entry = get_entry(virt_addr);

    if (!is_entry_valid(entry)) {
        println("virt_addr (%x) is not valid (has no entry)", virt_addr);
        return 0x0;
    }

    if (!is_entry_present(entry)) {
        println("virt_addr(%x) is not present", virt_addr);
        return 0x0;
    }

    return (*entry & ~((uint64_t)0xfff)) + (virt_addr & 0xfff);
}

void map_to(hPage page, hFrame frame, PageFlags flags) {
    uint64_t* entry = get_entry(page.start_addr);

    if (is_entry_valid(entry)) {
        *entry = frame.start_addr | flags;

        flush_TLB((void*) page.start_addr);
    } else {
        if (!(*entry & NOT_PRESENT)) { // MISSING_TABLE
            create_missing_tables(entry, page, frame, flags);
            flush_TLB((void*) page.start_addr);
            return;
        }

        println("A Page directory is not present -> could not get pt entry");
    }
}

bool is_addr_present(uint64_t virt_addr) {
    uint64_t* entry = get_entry(virt_addr);

    if (is_entry_valid(entry))
        return is_entry_present(entry);

    return false;
}

void show_entries(uint16_t ptEntries, uint16_t ptTables) {
    println("pml4_table addr %x", pml4_table);

    for (uint16_t i2 = 0; i2 < ptTables; i2++) {
        uint64_t* entry = get_entry_by_index(0, 0, i2, 0);
        println("pd_table[%d] %x", i2, *entry);

        for (uint16_t i1 = 0; i1 < ptEntries; i1++) {
            uint64_t* entry = get_entry_by_index(0, 0, i2, i1);
            println("  pt_table[%d] entry[%d] %x", i2, i1, *entry);
        }
    }
}

// virt         -> phys
// 0x2000       -> 0xb8000
// 0x800000     -> 0x3000
// 0xfffffff000 -> 0xfff000
void test_mapping() {
    clear_screen();
    // fixed
    // println("**<- flush_TLB clears the first two fields (first 32bit) of the console");

    // test how much is mapped (should be 8MiB)
    println("%x is present = %b", 0x7fffff, is_addr_present(0x7fffff));
    println("%x is present = %b", 0x800000, is_addr_present(0x800000));
    println("%x is present = %b", 0xfffffff000, is_addr_present(0xfffffff000));


    hFrame frame1 = get_hFrame(0xb8000);
    hFrame frame2 = get_hFrame(0x3000);
    hFrame frame3 = get_hFrame(0xffffff);

    hPage page1 = get_hPage(0x2000);
    hPage page2 = get_hPage(0x800000);
    hPage page3 = get_hPage(0xfffffff000);

    uint16_t* testAddr1 = (uint16_t*) 0x2000;
    uint16_t* testAddr2 = (uint16_t*) 0x800010;
    uint64_t* testAddr3 = (uint64_t*) 0xfffffffff0;

    println("before:");
    println("testAddr1 virt %x -> phys %x", (uint64_t) testAddr1, to_phys((uint64_t) testAddr1));
    println("testAddr2 virt %x -> phys %x", (uint64_t) testAddr2, to_phys((uint64_t) testAddr2));
    println("testAddr3 virt %x -> phys %x", (uint64_t) testAddr3, to_phys((uint64_t) testAddr3));
    println("testAddr1 val: %d ", *testAddr1);
    println("*testAddr2 would cause a page fault");

    map_to(page1, frame1, Present | Writeable);
    map_to(page2, frame2, Present | Writeable); // needs to allocate a new frame for a new PageTable
    map_to(page3, frame3, Present | Writeable); // needs to allocated several new frames for new PageTables

    println("after:");
    println("testAddr1 virt %x -> phys %x", (uint64_t) testAddr1, to_phys((uint64_t) testAddr1));
    println("testAddr2 virt %x -> phys %x", (uint64_t) testAddr2, to_phys((uint64_t) testAddr2));
    println("testAddr3 virt %x -> phys %x", (uint64_t) testAddr3, to_phys((uint64_t) testAddr3));

    for (uint16_t i = 24 * 80; i < 25 * 80; i++)
        *(testAddr1 + i) = WHITE << 12;

    *testAddr3 = 86;

    println("after '*testAddr3 = 86':");
    println("testAddr3 val: %d", *testAddr3);


    println("%x is present = %b", 0x800000, is_addr_present(0x800000));
    println("%x is present = %b", 0xfffffff000, is_addr_present(0xfffffff000));
}

