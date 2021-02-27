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
}

void show_entries( int ptEntries, int ptTables ) {
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

inline uint8_t get_lv4_index( uint64_t virt_addr ) {
    return ( uint8_t )( virt_addr >> 39 );
}

inline uint8_t get_lv3_index( uint64_t virt_addr ) {
    return ( uint8_t )( virt_addr >> 30 );
}

inline uint8_t get_lv2_index( uint64_t virt_addr ) { 
    return ( uint8_t )( virt_addr >> 21 );
}

inline uint8_t get_lv1_index( uint64_t virt_addr ) {
    return ( uint8_t )( virt_addr >> 12 );
}

uint64_t* get_entry( uint64_t virt_addr ) {
    uint8_t lv2Index = get_lv2_index( virt_addr );
    uint8_t lv1Index = get_lv1_index( virt_addr );

    uint64_t pt_addr = pd_table->pt_tables[lv2Index] & 0xfffff000;
    PTTable* pt_table = ( PTTable* )pt_addr;

    return &pt_table->entries[lv1Index];
}

inline uint64_t to_phys( uint64_t virt_addr ) {
    uint64_t* entry = get_entry( virt_addr );

    return ( *entry & 0xfffff000 ) + ( virt_addr & 0xfff ) + physical_offset;
}

inline void flush_TLB( void* m ) {
    __asm__ ( 
        "mov %%eax, (%0)\n"
	    "invlpg	(%%eax)\n"
        : : "b"(m) : "memory", "eax" 
    );
}

inline void map_to( hPage page, hFrame frame, PageFlags flags ) {
    *get_entry( *page.start_addr ) = *frame.start_addr | flags;

    flush_TLB( page.start_addr );
}

// 0x1000 -> 0xb8000
// 0x0    -> 0x2000
void test_mapping() {
    hFrame frame1 = get_hFrame( 0xb8000 );
    hFrame frame2 = get_hFrame( 0x2000 );
    
    hPage page1 = get_hPage( 0x1000 );
    hPage page2 = get_hPage( 0x0 );

    uint16_t* testAddr1 = ( uint16_t* )0x1000;
    uint16_t* testAddr2 = ( uint16_t* )0x10;

    // invlpg in flush_TLB uses eax register( 32bit )
    // -> clears 2x 16bit fields
    // -> clears 'b' and 'e'
    println( "before:( flush_TLB clears the 'be' of 'before' )" );
    println( "testAddr1 virt %x -> phys %x", ( uint64_t )testAddr1, to_phys( *testAddr1 ) );
    println( "testAddr2 virt %x -> phys %x", ( uint64_t )testAddr2, to_phys( *testAddr2 ) );

    map_to( page1, frame1, Present | Writeable );
    map_to( page2, frame2, Present | Writeable );

    println( "after:" );
    println( "testAddr1 virt %x -> phys %x", ( uint64_t )testAddr1, to_phys( *testAddr1 ) );
    println( "testAddr2 virt %x -> phys %x", ( uint64_t )testAddr2, to_phys( *testAddr2 ) );
    
    println( "" );
    
    println( "before:" );
    println( "testAddr1 val: %d ", *testAddr1 );
    println( "testAddr2 val: %d ", *testAddr2 );

    for ( uint16_t i = 6 * 80; i < 7 * 80; i++ )
        *( testAddr1 + i ) = WHITE << 12;
    
    *testAddr2 = 13;

    println( "after:" );
    println( "testAddr2 val: %d", *testAddr2 );
}