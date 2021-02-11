#![no_std]
#![feature(lang_items)]
#![feature(panic_info_message)]
#![allow(non_snake_case)]

use core::panic::PanicInfo;
use hBasicIO::AsciiColor;

extern crate rlibc;

mod hBasicIO;

#[no_mangle]
pub extern fn kernel_main() -> ! {

    hBasicIO::setConsoleColor( AsciiColor::Black, AsciiColor::LightGray );

    hBasicIO::clearConsole();

    println!( "{} to {}", "Welcome", "haiirOS" );

    panic!( "Test panic" ); 

    loop {}
}


#[lang = "eh_personality"] 
extern fn eh_personality() {}

#[panic_handler]
fn panic( info: &PanicInfo ) -> ! {
    hBasicIO::setConsoleColor( AsciiColor::Black, AsciiColor::Red );
    hBasicIO::printString( "Kernel paniced: " );
    
    if let Some( message ) = info.message() {
        printf!( "'{}', ", message );
    } else {
        hBasicIO::printString( "no details available, " );
    }

    if let Some( location ) = info.location() {
        printf!( "file '{}' line {}", location.file(), location.line() );
    } else {
        hBasicIO::printString( "file and line unknown" );
    }
    
    loop {}
}