#![no_std]
#![feature(lang_items)]
#![feature(panic_info_message)]
#![allow(non_snake_case)]

extern crate rlibc;

mod io;
mod memory;

use core::panic::PanicInfo;
use io::hBasicIO;
use io::hBasicIO::AsciiColor;
use memory::hMemoryMap::{ HBootInfo, HMemoryInfo };
use memory::hPaging::HPaging;
use memory::hPaging;

#[no_mangle]
pub extern fn kernel_main( multiboot_info_ptr: usize ) -> ! {
    hBasicIO::setConsoleColor( AsciiColor::Black, AsciiColor::LightGray );
    hBasicIO::clearConsole();
    println!( "Welcome to haiirOS" );

    let boot_info: HBootInfo = HBootInfo::new( multiboot_info_ptr );
    let mem_info: HMemoryInfo = HMemoryInfo::new( &boot_info );
    boot_info.printAddr();
    mem_info.print();

    let mut paging = unsafe{ HPaging::init( mem_info.phys_offset ) };
    let mut frame_allocator = memory::hFrameAllocator::MemoryMapFrames;

    // TODO: unit testing
    hPaging::testTranslating( &mut paging, mem_info.phys_offset );
    unsafe { hPaging::testMapping( &mut paging, &mut frame_allocator ); }


    loop {}
}


#[lang = "eh_personality"] 
extern fn eh_personality() {}

#[panic_handler]
fn panic( info: &PanicInfo ) -> ! {
    hBasicIO::setConsoleColor( AsciiColor::Black, AsciiColor::Red );
    printf!( "Kernel paniced: " );
    
    if let Some( message ) = info.message() {
        printf!( "'{}', ", message );
    } else {
        printf!( "no details available, " );
    }

    if let Some( location ) = info.location() {
        printf!( "file '{}' line {}", location.file(), location.line() );
    } else {
        printf!( "file and line unknown" );
    }
    
    loop {}
}