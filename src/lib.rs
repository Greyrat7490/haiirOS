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
use x86_64::VirtAddr;


#[no_mangle]
pub extern fn kernel_main( multiboot_info_ptr: usize ) -> ! {
    hBasicIO::setConsoleColor( AsciiColor::Black, AsciiColor::LightGray );
    hBasicIO::clearConsole();
    println!( "Welcome to haiirOS" );

    let boot_info: HBootInfo = HBootInfo::new( multiboot_info_ptr );
    let mem_info: HMemoryInfo = HMemoryInfo::new( &boot_info );
    mem_info.print();

    let paging = unsafe{ HPaging::init( mem_info.phys_offset ) };

    let addrs = [
        // identity-mapped so it should be 0x0 as well
        mem_info.phys_offset,
        0x0ff,
        0x0fff,
        0xb8000,
        0x9fc00,
        0x9ffff,
        0x7fffff,
        0x800000,
        0x1000000,
        0xffffffff
    ];
        
    for i in 0..addrs.len() {
        let testPhys = paging.to_Phys( VirtAddr::new( addrs[i] ) );
        println!( "virt: {:#x} to {:?}", addrs[i], testPhys );
    }

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