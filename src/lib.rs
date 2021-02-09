#![no_std]
#![feature(lang_items)]
#![allow(non_snake_case)]

use core::panic::PanicInfo;
use hBasicIO::AsciiColor;

extern crate rlibc;

mod hBasicIO;

#[no_mangle]
pub extern fn kernel_main() -> ! {

    hBasicIO::setConsoleColor( AsciiColor::Black, AsciiColor::LightGray );

    hBasicIO::clearConsole();

    hBasicIO::printString( "Welcome to haiirOS" );

    loop {}
}


#[lang = "eh_personality"] 
extern fn eh_personality() {}

#[panic_handler]
fn panic( _info: &PanicInfo ) -> ! {
    hBasicIO::printString( "Err: PANIC!!!!" );
    loop {}
}