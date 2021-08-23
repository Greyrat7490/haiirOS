#include "interrupt/ISR/isr.h"
#include "memory/Paging/Frame/hFrame.h"
#include "memory/Paging/hPaging.h"
#include "proc/task.h"
#include "types.h"
#include "io/io.h"
#include "memory/memory.h"
#include "interrupt/idt.h"

extern void test_user_function() {
    while(1){};
}

extern void jump_usermode();

void kernel_main( uint64_t boot_info_addr ) {
    clear_screen();
    set_color( BLACK, PINK );

    println( "%s to %s!", "Welcome", "haiirOS" );

    hMemoryMap mmap = init_memory_map( boot_info_addr );
    init_paging();

    select_keyboard_layout( QWERTZ_LAYOUT );
    
    init_idt( 0x108000 ); // IDT from 0x108000 - 0x110000
    
    clear_screen();
    
    // test breakpoint interrupt
    /* __asm__ volatile ( "int $0x3" ); */

    bool is_present = is_addr_present( 0x7fffff );
    bool is_present2 = is_addr_present( 0x800000 );

    println( "%x is present = %d", 0x7fffff, is_present );
    println( "%x is present = %d", 0x800000, is_present2 );

    println( "test_usr_func addr: %x", (uint64_t)test_user_function );
    
    // TODO: create paging tabels for user/task and load into cr3
 /*    hFrame test_func_frame = get_hFrame( (uint64_t)test_user_function );
  *    hPage test_func_page = get_hPage( (uint64_t)test_user_function );
  *    map_to( test_func_page, test_func_frame, Present | Writeable | User );
  *
  *    // usr_stack 0x121000 - 0x11b000
  *    extern uint64_t usr_stack;
  *    println("user stack addr top: %x", usr_stack);
  *    println("user stack addr bottom: %x", usr_stack - 4096 * 6);
  *
  *    for ( int i = 0; i < 7; i++ ) {
  *        hFrame frame = get_hFrame( usr_stack - i * 0x1000 );
  *        hPage page = get_hPage( usr_stack - i * 0x1000 );
  *        map_to( page, frame, Present | Writeable | User );
  *    } */

    init_tss();
    
    println("going into user mode...");
    jump_usermode();
    
    while(1) __asm__("hlt");
}
