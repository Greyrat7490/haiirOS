#![no_std]
#![feature(lang_items)]
#![feature(panic_info_message)]
#![allow(non_snake_case)]

extern crate rlibc;

mod IO;
mod memory;

use core::panic::PanicInfo;
use IO::hBasicIO::{ AsciiColor, setConsoleColor, clearConsole, printString };
use memory::hMemoryMap::{ HBootInfo, HMemoryInfo };


#[no_mangle]
pub extern fn kernel_main( multiboot_info_ptr: usize ) -> ! {

    setConsoleColor( AsciiColor::Black, AsciiColor::LightGray );

    clearConsole();

    println!( "{} to {}", "Welcome", "haiirOS" );

    let boot_info: HBootInfo = HBootInfo::new( multiboot_info_ptr );

    let mem_info: HMemoryInfo = HMemoryInfo::new( &boot_info );

    mem_info.print();

    loop {}
}


#[lang = "eh_personality"] 
extern fn eh_personality() {}

#[panic_handler]
fn panic( info: &PanicInfo ) -> ! {
    setConsoleColor( AsciiColor::Black, AsciiColor::Red );
    printString( "Kernel paniced: " );
    
    if let Some( message ) = info.message() {
        printf!( "'{}', ", message );
    } else {
        printString( "no details available, " );
    }

    if let Some( location ) = info.location() {
        printf!( "file '{}' line {}", location.file(), location.line() );
    } else {
        printString( "file and line unknown" );
    }
    
    loop {}
}