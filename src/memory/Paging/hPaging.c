#include "hPaging.h"
#include "io/hBasicIO.h"

typedef struct
{
    uint64_t entries[512];
} PTTable;

typedef struct
{
    uint64_t pt_tables[4];
} PDTable;

typedef struct
{
    uint64_t pd_table;
} PDPTable;

typedef struct
{
    uint64_t pdp_table;
} PML4Table;


static PML4Table* pml4_table;
static PDPTable* pdp_table;
static PDTable* pd_table;

uint32_t physical_offset = 0;


void init_paging() {
    uint64_t addr;

    __asm__ (
        "mov %%cr3, %%rax\n" // cr3 -> rax
        "mov %%rax, %0\n"   // rax -> addr
        : "=m" ( addr )
        : // no input
        : "%rax"
    );

    println( "physical pml4 addr: %x", addr );
    addr += physical_offset;
    println( "virtual pml4 addr: %x", addr );

    // PML4-Table
    pml4_table = ( PML4Table* )addr;

    // PDP-Table
    uint64_t pdp_addr = pml4_table->pdp_table & 0xfffff000; //set last 12bits( only flags ) to 0
    pdp_table = ( PDPTable* )pdp_addr;

    // PD-Table
    uint64_t pd_addr = pdp_table->pd_table & 0xfffff000;
    pd_table = ( PDTable* )pd_addr;


    // for testing mapping 0x1000 -> 0xb8000
                        // 0x0    -> 0x2000
    uint64_t pt_addr = pd_table->pt_tables[0] & 0xfffff000;
    PTTable* tmp = ( PTTable* )pt_addr;

    tmp->entries[1] = 0xb8000 | 3; // writable + present
    tmp->entries[0] = 0x2000 | 3;
    
    flushTLB( 0x0 );
    flushTLB( 0x1000 );

    printf("");
}

void showEntries( int ptEntries, int ptTables ) {
    println( "pml4_table addr %x", pml4_table );
    println( "pdp_table addr %x", pdp_table );
    println( "pd_table addr %x", pd_table );
    
    for ( int i = 0; i < ptTables; i++ ) {
        // PT-Table
        uint64_t pt_addr = pd_table->pt_tables[i] & 0xfffff000;
        PTTable* pt_table = ( PTTable* )pt_addr;
        println( "pt_table[%d] addr %x", i, pt_table );

        for ( int j = 0; j < ptEntries; j++ ) {
            // PT-Table Entry
            uint64_t entry = pt_table->entries[j] & 0xfffff000;
            println( "pt_table[%d] entry[%d] %x", i, j, entry );
        }
    }
}

uint8_t getLv4Index( uint64_t virt_addr ) {
    return ( uint8_t )( virt_addr >> 39 );
}

uint8_t getLv3Index( uint64_t virt_addr ) {
    return ( uint8_t )( virt_addr >> 30 );
}

uint8_t getLv2Index( uint64_t virt_addr ) { 
    return ( uint8_t )( virt_addr >> 21 );
}

uint8_t getLv1Index( uint64_t virt_addr ) {
    return ( uint8_t )( virt_addr >> 12 );
}

uint64_t to_phys( uint64_t virt_addr ) {
    uint8_t lv2Index = getLv2Index( virt_addr );
    uint8_t lv1Index = getLv1Index( virt_addr );

    uint64_t pt_addr = pd_table->pt_tables[lv2Index] & 0xfffff000;
    PTTable* pt_table = ( PTTable* )pt_addr;

    uint64_t res = pt_table->entries[lv1Index] & 0xfffff000;
    res += virt_addr & 0xfff;
    res += physical_offset;

    return res;
}

void flushTLB( void* m ) {
    __asm__ ( 
        "mov %%eax, (%0)\n"
	    "invlpg	(%%eax)\n"
        : : "b"(m) : "memory", "eax" 
    );
}