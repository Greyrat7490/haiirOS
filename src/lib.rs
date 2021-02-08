#![no_std]
#![feature(lang_items)]
#![allow(non_snake_case)]

use core::panic::PanicInfo;

extern crate rlibc;

const CONSOLE_WIDTH: usize = 80;
const CONSOLE_HEIGHT: usize = 25;

#[no_mangle]
pub extern fn kernel_main() -> ! {
    let vga_buffer;
    
    unsafe { // dereferencing a pointer is unsafe
        // at memory Address 0xb8000 is the VGA text buffer
        vga_buffer = &mut *( 0xb8000 as *mut [u16; CONSOLE_WIDTH * CONSOLE_HEIGHT] );
    }

    // clear screen ( all black )
    for i in 0..CONSOLE_WIDTH * CONSOLE_HEIGHT {
        vga_buffer[i] = 0x0000;
    }
    
    // write "OS" ( in grey ) to the screen
    vga_buffer[0] = 0x074f; // 0 -> black bg, 7 -> grey fg, 4f -> 'O'
    vga_buffer[1] = 0x0753; // 0 -> black bg, 7 -> grey fg, 53 -> 'S'

    loop {}
}


#[lang = "eh_personality"] 
extern fn eh_personality() {}

#[panic_handler]
fn panic( _info: &PanicInfo ) -> ! {
    loop {}
}