#include "paging.h"

#include "types.h"
#include "io/io.h"
#include "phys.h"
#include "virt.h"


typedef struct {
    uint64_t entries[512];
} PTTable;

typedef struct {
    uint64_t pt_tables[512];
} PDTable;

typedef struct {
    uint64_t pd_tables[512];
} PDPTable;

typedef struct {
    uint64_t pdp_tables[512];
} PML4Table;


static PML4Table* s_pml4_table;

#define TMP_ADDR 0x200000

static bool is_entry_valid(uint64_t* entry) {
    return entry != 0x0;
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
    __asm__ volatile ("invlpg (%0)" : : "r"(m) : "memory");
}

static uint64_t* get_temp_entry(void) {
    uint64_t virt_addr = TMP_ADDR;
    uint16_t i1 = get_lv1_index(virt_addr);
    uint16_t i2 = get_lv2_index(virt_addr);
    uint16_t i3 = get_lv3_index(virt_addr);
    uint16_t i4 = get_lv4_index(virt_addr);

    uint64_t* pml4_entry = &s_pml4_table->pdp_tables[i4];

    PDPTable* pdp_table = (PDPTable*) (*pml4_entry & ~0xfff);
    uint64_t* pdp_entry = &pdp_table->pd_tables[i3];

    PDTable* pd_table = (PDTable*) (*pdp_entry & ~0xfff);
    uint64_t* pd_entry = &pd_table->pt_tables[i2];

    PTTable* pt_table = (PTTable*) (*pd_entry & ~0xfff);
    return &pt_table->entries[i1];
}

static uint64_t* tmp_map(uint64_t phys_addr) {
    uint64_t* tmp_entry = get_temp_entry();

    frame_t frame = to_frame(phys_addr);
    *tmp_entry = frame | Present | Writeable;

    flush_TLB((void*) TMP_ADDR);
    return (uint64_t*) (TMP_ADDR + (phys_addr & 0xfff));
}

// return virtual address to the table
static uint64_t* create_table(uint64_t* entry, PageFlags flags) {
    void* table = pmm_alloc_unmapped(1);
    *entry = (uint64_t)table | flags;
    return tmp_map((uint64_t)table);
}

// if pointer is not dereferenceable it will
// return a pointer with a temporary mapped virtual address
// TODO: virtual addr allocator (to keep track of mapped addresses)
static void* valid_pointer(void* pointer) {
    // temporary way to check if an address is dereferenceable
    // without causing a page fault if not
    if ((uint64_t) pointer >= 0x800000)
        return tmp_map((uint64_t) pointer);

    return pointer;
}

static uint64_t* get_table_from_entry(uint64_t* entry, PageFlags new_table_flags) {
    entry = (uint64_t*) valid_pointer(entry);

    uint64_t* table = (uint64_t*) (*entry & ~0xfff);
    if (!is_entry_present(entry)) {
        if ((uint64_t)table == 0 || pmm_is_free((uint64_t)table)) {
            if (new_table_flags != (uint64_t)-1) {
                return create_table(entry, new_table_flags);
            } else {
                return 0x0;
            }
        }
    }

    return valid_pointer(table);
}

static uint64_t* get_entry_by_index(PML4Table* pml4_table, PageFlags new_table_flags, uint16_t i4, uint16_t i3, uint16_t i2, uint16_t i1) {
    uint64_t* pml4_entry = &pml4_table->pdp_tables[i4];
    PDPTable* pdp_table = (PDPTable*) get_table_from_entry(pml4_entry, new_table_flags);
    if (pdp_table == 0x0) { return 0x0; }

    uint64_t* pdp_entry = &pdp_table->pd_tables[i3];
    PDTable* pd_table = (PDTable*) get_table_from_entry(pdp_entry, new_table_flags);
    if (pdp_table == 0x0) { return 0x0; }

    uint64_t* pd_entry = &pd_table->pt_tables[i2];
    PTTable* pt_table = (PTTable*) get_table_from_entry(pd_entry, new_table_flags);
    if (pdp_table == 0x0) { return 0x0; }

    return &pt_table->entries[i1];
}

static uint64_t* lookup_entry_by_index(PML4Table* pml4_table, uint16_t i4, uint16_t i3, uint16_t i2, uint16_t i1) {
    return get_entry_by_index(pml4_table, (uint64_t)-1, i4, i3, i2, i1);
}

static uint64_t* get_entry(PML4Table* pml4_table, PageFlags new_table_flags, uint64_t virt_addr) {
    uint16_t i1 = get_lv1_index(virt_addr);
    uint16_t i2 = get_lv2_index(virt_addr);
    uint16_t i3 = get_lv3_index(virt_addr);
    uint16_t i4 = get_lv4_index(virt_addr);

    return get_entry_by_index(pml4_table, new_table_flags, i4, i3, i2, i1);
}

// like get_entry but does not create a new table if needed (returns 0x0 instead)
static uint64_t* lookup_entry(PML4Table* pml4_table, uint64_t virt_addr) {
    return get_entry(pml4_table, (uint64_t)-1, virt_addr);
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
    uint64_t* entry = lookup_entry(s_pml4_table, virt_addr);

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
    uint64_t* entry = lookup_entry((PML4Table*) pml4_table, virt_addr);

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
    uint64_t* entry = get_entry((PML4Table*) pml4_table, flags, page);

    if (is_entry_valid(entry)) {
        *entry = frame | flags;

        flush_TLB((void*) page);
    } else {
        kprintln("ERROR: could not get pt entry");
    }
}

static void unmap(uint64_t* pml4_table, page_t page) {
    uint64_t* entry = lookup_entry((PML4Table*) pml4_table, page);

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
    uint64_t* entry = lookup_entry(s_pml4_table, virt_addr);

    if (is_entry_valid(entry))
        return is_entry_present(entry);

    return false;
}

bool is_usr_addr_present(uint64_t* pml4_table, uint64_t virt_addr) {
    uint64_t* entry = lookup_entry((PML4Table*) pml4_table, virt_addr);

    if (is_entry_valid(entry))
        return is_entry_present(entry);

    return false;
}

void show_entries(uint16_t ptEntries, uint16_t ptTables) {
    kprintln("pml4_table addr %x", s_pml4_table);

    for (uint16_t i2 = 0; i2 < ptTables; i2++) {
        uint64_t* entry = lookup_entry_by_index(s_pml4_table, 0, 0, i2, 0);
        kprintln("pd_table[%d] %x", i2, *entry);

        for (uint16_t i1 = 0; i1 < ptEntries; i1++) {
            uint64_t* entry = lookup_entry_by_index(s_pml4_table, 0, 0, i2, i1);
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

