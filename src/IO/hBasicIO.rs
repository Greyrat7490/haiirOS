#![allow(dead_code)]

use core::fmt::Arguments;
use core::fmt::Write;
use spin::Lazy;
use spin::Mutex;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum AsciiColor {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    Pink = 13,
    Yellow = 14,
    White = 15,
}

const CONSOLE_WIDTH: usize = 80;
const CONSOLE_HEIGHT: usize = 25;

struct VGABuffer {
    buffer: &'static mut [u16; CONSOLE_WIDTH * CONSOLE_HEIGHT],
    x: usize,
    y: usize,
    color: u8
}

impl VGABuffer {
    fn printByte( &mut self, c: u8 ) {
        let mut field: u16 = self.color as u16;

        match c {
            b'\n' => {
                self.x = 0;
                self.y += 1;

                if self.y >= CONSOLE_HEIGHT {
                    self.x = 0;
                    self.y = 0;
                }
            },
            _ => {
                field = field << 8 | c as u16;
                
                if self.x >= CONSOLE_WIDTH {
                    self.x = 0;
                    self.y += 1;
                }
                
                if self.y >= CONSOLE_HEIGHT {
                    self.x = 0;
                    self.y = 0;
                }
                
                self.buffer[self.x + self.y * CONSOLE_WIDTH] = field;
                
                self.x += 1;
            }
        }
    }  

    fn printString( &mut self, string: &str ) {
        for b in string.bytes() {
            self.printByte( b );
        }
    }

    fn setColor( &mut self, bg: AsciiColor, fg: AsciiColor ) {
        self.color = ( bg as u8 ) << 4 | fg as u8;
    }

    fn clear( &mut self ) {
        self.x = 0;
        self.y = 0;

        for _i in 0..CONSOLE_WIDTH * CONSOLE_HEIGHT {
            self.printByte( b' ' );
        }
    }
}

impl Write for VGABuffer {
    fn write_str( &mut self, s: &str ) -> core::fmt::Result {
        self.printString( s );
        Ok(())
    }
}

static VGA_BUFFER: Lazy<Mutex<VGABuffer>> = Lazy::new( ||
    Mutex::new(
        VGABuffer {
            buffer: unsafe{ &mut *( 0xb8000 as *mut [u16; CONSOLE_WIDTH * CONSOLE_HEIGHT] ) },
            x: 0,
            y: 0,
            color: ( AsciiColor::Black as u8 ) << 4 | AsciiColor::Pink as u8
        }  
    )
);


pub fn printByte( b: u8 ) {
    VGA_BUFFER.lock().printByte( b );
}

pub fn printChar( c: char ) {
    VGA_BUFFER.lock().printByte( c as u8 );
}

pub fn printString( string: &str ) {
    VGA_BUFFER.lock().printString( string );
}

pub fn clearConsole() {
    VGA_BUFFER.lock().clear();
}

pub fn setConsoleColor( bg: AsciiColor, fg: AsciiColor ) {
    VGA_BUFFER.lock().setColor( bg, fg );
}

pub fn printfmt( args: Arguments ) {
    VGA_BUFFER.lock().write_fmt( args ).unwrap();
}

#[macro_export]
macro_rules! printf {
    ( $( $args:tt )* ) => ( crate::io::hBasicIO::printfmt( format_args!( $( $args )* ) ) );
}

#[macro_export]
macro_rules! println {
    () => ( hBasicIO::printByte( b'\n' ) );
    ( $( $args:tt )* ) => ( 
        printf!( $( $args )* );
        crate::io::hBasicIO::printByte( b'\n' );
    );
}