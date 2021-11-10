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
    uint64_t pd_table[512];
} PDPTable;

typedef struct
{
    uint64_t pdp_table[512];
} PML4Table;


static PML4Table* pml4_table;


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

// TODO: proper optional
// if PTTable does not exist:
//    - returns a Dirty, Accessed and not present addr
static uint64_t* get_entry_by_index(uint16_t pml4_index, uint16_t pdp_index, uint16_t pd_index, uint16_t pt_index) {
    uint64_t pml4_entry = pml4_table->pdp_table[pml4_index];
    if ((pml4_entry & 1) == 0) // check if entry is present
        return (uint64_t*) (Dirty | Accessed);

    PDPTable* pdp_table = (PDPTable*) (pml4_entry & 0xfffff000);


    uint64_t pdp_entry = pdp_table->pd_table[pdp_index];
    if ((pdp_entry & 1) == 0)
        return (uint64_t*) (Dirty | Accessed);

    PDTable* pd_table = (PDTable*) (pdp_entry & 0xfffff000);


    uint64_t pd_entry = pd_table->pt_tables[pd_index];
    if ((pd_entry & 1) == 0)
        return (uint64_t*) (Dirty | Accessed);


    PTTable* pt_table = (PTTable*) (pd_entry & 0xfffff000);

    return &pt_table->entries[pt_index];
}

static uint64_t* get_entry(uint64_t virt_addr) {
    uint16_t i1 = get_lv1_index(virt_addr);
    uint16_t i2 = get_lv2_index(virt_addr);
    uint16_t i3 = get_lv3_index(virt_addr);
    uint16_t i4 = get_lv4_index(virt_addr);

    return get_entry_by_index(i4, i3, i2, i1);
}

static bool is_entry_valid(uint64_t* entry) {
    return !((uint64_t) entry == (Dirty | Accessed));
}

static bool is_entry_present(uint64_t* entry) {
    return *entry & Present;
}

static void create_missing_table(uint64_t* entry, uint64_t virt_addr, PageFlags flags) {
    hFrame frame = alloc_frame();

    // set entry to table
    *entry = frame.start_addr | flags;

    // DOC:
    // I intended to use 0x0, because the page is already unmapped
    // and the address is usually unusable (so perfect for temporary mapping).
    // For this reason, gcc unfortunately optimises this out in 02/O3 (causing invalid_opcode).
    // 0x0(nullptr) is not a valid address and if you use it, it can always be optimised out.

    // temporary map the frame to 0x1000 to set the table
    map_to(get_hPage(0x1000), frame, Present | Writeable);

    // set table
    uint64_t* destAddr = (uint64_t*) 0x1000;
    *destAddr = virt_addr | flags;
}

// virt_addr: address with missing tables
static void create_missing_tables(uint64_t virt_addr, PageFlags flags) {
    uint16_t i2 = get_lv2_index(virt_addr);
    uint16_t i3 = get_lv3_index(virt_addr);
    uint16_t i4 = get_lv4_index(virt_addr);

    uint64_t* pml4_entry = &pml4_table->pdp_table[i4];

    // create missing pdptable
    if ((*pml4_entry & Present) == 0) {
      println("TODO: implement");
      while(1){}
      // create_missing_table(pml4_entry, virt_addr, flags);
    }

    PDPTable* pdp_table = (PDPTable*) (*pml4_entry & 0xfffff000);
    uint64_t* pdp_entry = &pdp_table->pd_table[i3];

    // create missing pdtable
    if ((*pdp_entry & Present) == 0) {
      println("TODO: implement");
      while(1){}
      // create_missing_table(pdp_entry, virt_addr, flags);
    }

    PDTable* pd_table = (PDTable*)(*pdp_entry & 0xfffff000);
    uint64_t* pd_entry = &pd_table->pt_tables[i2];

    // create missing pttable
    if ((*pd_entry & Present) == 0)
      create_missing_table(pd_entry, virt_addr, flags);
}

static void flush_TLB(void* m) {
    // invlpg uses eax register (32bit)
    // = 2x 16bit fields
    // -> sets "random" values for the first two fields of the console (addr of m)

    __asm__ (
        "mov %%eax, (%0)\n"
        "invlpg (%%eax)\n"
        : : "b"(m) : "memory", "eax" 
    );
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

    pml4_table = (PML4Table*)addr;
}

// TODO: proper optinal instead of returning 0xdeadbeef if virt_addr is not valid
uint64_t to_phys(uint64_t virt_addr) {
    uint64_t* entry = get_entry(virt_addr);

    if (!is_entry_valid(entry)) {
        println("virt_addr (%x) is not valid (has no entry)", virt_addr);
        return 0xdeadbeef;
    }

    if (!is_entry_present(entry)) {
        println("virt_addr(%x) is not present", virt_addr);
        return 0xdeadbeef;
    }

    return (*entry & 0xfffff000) + (virt_addr & 0xfff);
}

void map_to(hPage page, hFrame frame, PageFlags flags) {
    uint64_t* entry = get_entry(page.start_addr);

    if (is_entry_valid(entry)) {
        *entry = frame.start_addr | flags;

        flush_TLB((void*) page.start_addr);
    } else {
        create_missing_tables(page.start_addr, flags);
        println("New frame allocated for a new Page Table (for %x)", page.start_addr);
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
// 0x800000     -> 0x2000
void test_mapping() {
    clear_screen();
    println("**<- flush_TLB clears the first two fields (first 32bit) of the console");
    
    // test how much is mapped (should be 8MiB)
    println("%x is present = %b", 0x7fffff, is_addr_present(0x7fffff));
    println("%x is present = %b", 0x800000, is_addr_present(0x800000));


    hFrame frame1 = get_hFrame(0xb8000);
    hFrame frame2 = get_hFrame(0x2000);

    hPage page1 = get_hPage(0x2000);
    hPage page2 = get_hPage(0x800000);

    uint16_t* testAddr1 = (uint16_t*) 0x2000;
    uint16_t* testAddr2 = (uint16_t*) 0x800010;

    println("before:");
    println("testAddr1 virt %x -> phys %x", (uint64_t) testAddr1, to_phys((uint64_t) testAddr1));
    println("testAddr2 virt %x -> phys %x", (uint64_t) testAddr2, to_phys((uint64_t) testAddr2));
    println("testAddr1 val: %d ", *testAddr1);
    println("*testAddr2 would cause a page fault");

    map_to(page1, frame1, Present | Writeable);
    map_to(page2, frame2, Present | Writeable); // needs to allocate a new frame for a new PageTable

    println("after:");
    println("testAddr1 virt %x -> phys %x", (uint64_t) testAddr1, to_phys((uint64_t) testAddr1));
    println("testAddr2 virt %x -> phys %x", (uint64_t) testAddr2, to_phys((uint64_t) testAddr2));
    println("testAddr1 val: %d ", *testAddr1);
    println("testAddr2 val: %d ", *testAddr2);

    for (uint16_t i = 24 * 80; i < 25 * 80; i++)
        *(testAddr1 + i) = WHITE << 12;

    *testAddr2 = 13;

    println("after '*testAddr2 = 13':");
    println("testAddr2 val: %d", *testAddr2);


    println("%x is present = %b", 0x7fffff, is_addr_present(0x7fffff));
    println("%x is present = %b", 0x800000, is_addr_present(0x800000));
}

