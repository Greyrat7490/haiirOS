#include "hPaging.h"
#include "io/io.h"

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

inline uint16_t get_lv4_index( uint64_t virt_addr ) {
    return ( uint16_t )( virt_addr >> 39 );
}

inline uint16_t get_lv3_index( uint64_t virt_addr ) {
    return ( uint16_t )( virt_addr >> 30 );
}

inline uint16_t get_lv2_index( uint64_t virt_addr ) { 
    return ( uint16_t )( virt_addr >> 21 );
}

inline uint16_t get_lv1_index( uint64_t virt_addr ) {
    return ( uint16_t )( virt_addr >> 12 );
}

uint64_t* get_entry( uint64_t virt_addr ) {
    uint16_t i1 = get_lv1_index( virt_addr );
    uint16_t i2 = get_lv2_index( virt_addr );
    uint16_t i3 = get_lv3_index( virt_addr );
    uint16_t i4 = get_lv4_index( virt_addr );

    uint64_t pt_addr = pd_table->pt_tables[i2] & 0xfffff000;
    PTTable* pt_table = ( PTTable* )pt_addr;

    if( i1 >= sizeof( pt_table->entries ) / sizeof( uint64_t ) ) {
        return 0x0;
    }
    if( i2 >= sizeof( pd_table->pt_tables ) / sizeof( uint64_t ) ) {
        return 0x0;
    }
    if( i3 >= sizeof( pdp_table->pd_table ) / sizeof( uint64_t ) ) {
        return 0x0;
    }
    if( i4 >= sizeof( pml4_table->pdp_table ) / sizeof( uint64_t ) ) {
        return 0x0;
    }

    return &pt_table->entries[i1];
}

inline uint64_t to_phys( uint64_t virt_addr ) {
    uint64_t* entry = get_entry( virt_addr );

    return ( *entry & 0xfffff000 ) + ( virt_addr & 0xfff ) + physical_offset;
}

void flush_TLB( void* m ) {
    // just for testing -------------
    uint32_t volatile tmp = *( uint32_t* )m; // the "random" value
    *( volatile uint32_t* )m = 0;
    tmp = *( uint32_t* )m;
    // ------------------------------

    // invlpg in uses eax register( 32bit )
    // = 2x 16bit fields
    // -> clears( "random" value ) 'b' and 'e'
    __asm__ ( 
        "mov %%eax, (%0)\n"
	    "invlpg	(%%eax)\n"
        : : "b"(m) : "memory", "eax" 
    );
}

bool is_entry_present( uint64_t* entry ) {
    return *entry & Present;
}

inline void map_to( hPage page, hFrame frame, PageFlags flags ) {
    uint64_t* entry = get_entry( page.start_addr );

    if ( entry != 0x0 ) {
        *entry = frame.start_addr | flags;

        flush_TLB( ( void* )page.start_addr );
    } else {
        println( "Err: needs to allocate a new frame for a new pageTable", page.start_addr );
    }
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

    println( "before:( flush_TLB clears the 'be' of 'before' )" );
    println( "testAddr1 virt %x -> phys %x", ( uint64_t )testAddr1, to_phys( ( uint64_t )testAddr1 ) );
    println( "testAddr2 virt %x -> unmapped", ( uint64_t )testAddr2 );
    println( "testAddr1 val: %d ", *testAddr1 );
    //println( "testAddr2 val: %d ", *testAddr2 ); // would cause an error

    map_to( page1, frame1, Present | Writeable );
    map_to( page2, frame2, Present | Writeable );
    map_to( get_hPage( 0xffffffff ), frame2, Present | Writeable );

    println( "after:" );
    println( "testAddr1 virt %x -> phys %x", ( uint64_t )testAddr1, to_phys( ( uint64_t )testAddr1 ) );
    println( "testAddr2 virt %x -> phys %x", ( uint64_t )testAddr2, to_phys( ( uint64_t )testAddr2 ) );
    println( "testAddr1 val: %d ", *testAddr1 );
    println( "testAddr2 val: %d ", *testAddr2 );

    for ( uint16_t i = 24 * 80; i < 25 * 80; i++ )
        *( testAddr1 + i ) = WHITE << 12;
    
    *testAddr2 = 13;

    println( "after:" );
    println( "testAddr2 val: %d", *testAddr2 );
}