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

void init_paging( uint64_t phys_offset ) {
    uint64_t addr;

    __asm__ (
        "mov %%cr3, %%rax\n" // cr3 -> rax
        "mov %%rax, %0\n"   // rax -> addr
        : "=m" ( addr )
        : // no input
        : "%rax"
    );

    println( "physical pml4 addr: %x", addr );
    addr += phys_offset;
    println( "virtual pml4 addr: %x", addr );

    pml4_table = ( PML4Table* )addr;
}


void showEntries( int ptEntries, int ptTables ) {
    // PML4-Table
    println( "pml4_table addr %x", pml4_table );

    // PDP-Table
    uint64_t pdp_addr = pml4_table->pdp_table & 0xFFFFF000; //set last 12bits( only flags ) to 0
    PDPTable* pdp_table = ( PDPTable* )pdp_addr;
    println( "pdp_table addr %x", pdp_table );
    
    // PD-Table
    uint64_t pd_addr = pdp_table->pd_table & 0xFFFFF000;
    PDTable* pd_table = ( PDTable* )pd_addr;
    println( "pd_table addr %x", pd_table );
    
    for ( int i = 0; i < ptTables; i++ ) {
        // PT-Table
        uint64_t pt_addr = pd_table->pt_tables[i] & 0xFFFFF000;
        PTTable* pt_table = ( PTTable* )pt_addr;
        println( "pt_table[%d] addr %x", i, pt_table );

        for ( int j = 0; j < ptEntries; j++ ) {
            // PT-Table Entry
            uint64_t entry = pt_table->entries[j] & 0xFFFFF000;
            println( "pt_table[%d] entry[%d] %x", i, j, entry );
        }
    }
}