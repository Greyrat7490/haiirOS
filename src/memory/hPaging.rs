#![allow(dead_code)]

use crate::printf;
use crate::println;

use x86_64::{
    PhysAddr,
    VirtAddr
};

use x86_64::structures::paging::{
    PageTable,
    OffsetPageTable,
};

use super::hFrameAllocator::MemoryMapFrames;

pub struct HPaging {
    pub offset_page_table: OffsetPageTable<'static>,
    phys_offset: u64
}

impl HPaging {
    pub unsafe fn init( phys_offset: u64 ) -> Self {
        let pml4_table = getPML4Table( phys_offset );

        Self {
            offset_page_table: OffsetPageTable::new( pml4_table,
                VirtAddr::new( phys_offset ) ),
            phys_offset
        }
    }

    pub fn to_Phys( &self, addr: VirtAddr ) -> Option<PhysAddr> {
        use x86_64::structures::paging::Translate;
        
        self.offset_page_table.translate_addr( addr )
    }

    pub unsafe fn show_entries( &mut self ) {
        let pml4_table = self.offset_page_table.level_4_table();

        for ( i, e ) in pml4_table.iter().enumerate() {
            if !e.is_unused() {
                println!( "pml4 entry {}:\n  {:?} flags: {:?}", i, e.addr(), e.flags() );
    
                let phys = e.frame().unwrap().start_address();
                let virt = phys.as_u64() + self.phys_offset;
                let ptr = VirtAddr::new( virt ).as_mut_ptr();
                let l3_table: &PageTable = &*ptr;
            
                // print non-empty entries of the level 3 table
                for (i, e) in l3_table.iter().enumerate() {
                    if !e.is_unused() {
                        println!("L3 Entry {}: {:?}", i, e);
    
                        let phys = e.frame().unwrap().start_address();
                        let virt = phys.as_u64() + self.phys_offset;
                        let ptr = VirtAddr::new( virt ).as_mut_ptr();
                        let l2_table: &PageTable = &*ptr;
    
                        for (i, e) in l2_table.iter().enumerate() {
                            if !e.is_unused() {
                                println!("L2 Entry {}: {:?}", i, e);
    
                                let phys = e.frame().unwrap().start_address();
                                let virt = phys.as_u64() + self.phys_offset;
                                let ptr = VirtAddr::new( virt ).as_mut_ptr();
                                let l1_table: &PageTable = &*ptr;
    
                                for (i, e) in l1_table.iter().enumerate() {
                                    if !e.is_unused() {
                                        println!("L1 Entry {}: {:?}", i, e);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

unsafe fn getPML4Table( phys_offset: u64 ) -> &'static mut PageTable {
    use x86_64::registers::control::Cr3;

    let ( pml4_table, _ ) = Cr3::read();

    let phys_addr = pml4_table.start_address().as_u64();
    let virt_addr = phys_offset + phys_addr;
    let tablePtr = virt_addr as *mut PageTable;

    &mut *tablePtr
}

/// test mapping by mapping the first page to the VGA Buffer Frame
/// and printing a gray line to see it workes 
pub unsafe fn testMapping( paging: &mut HPaging, frame_allocator: &mut MemoryMapFrames ) {
    // first page ( 0x0 - 0xfff ) is unused
    let page = Page::<Size4KiB>::containing_address( VirtAddr::new( 0x0 ) );
    
    use x86_64::structures::paging::{ 
        PageTableFlags,
        Mapper,
        Page,
        PhysFrame,
        Size4KiB 
    };

    use crate::io::hBasicIO::AsciiColor;

    // mapping frame with VGA Buffer for testing
    let frame = PhysFrame::containing_address( PhysAddr::new( 0xb8000 ) );
    let flags = PageTableFlags::PRESENT | PageTableFlags::WRITABLE;

    let res = paging.offset_page_table.map_to( page, frame, flags, frame_allocator );
    res.expect( "map_to failed" ).flush();

    let testPhys = paging.to_Phys( VirtAddr::new( 0x0 ) ).unwrap();
    println!( "test mapping:" );
    println!( "  virt: {:#x} to {:?}", 0x0, testPhys );

    // print gray line to see it workes
    let test_vga_buffer: &mut [u16; 80] = &mut *( 0x0 as *mut [u16; 80] );
    for i in 20..test_vga_buffer.len() - 20 {
        test_vga_buffer[i] = ( AsciiColor::DarkGray as u16 ) << 12;
    }
}

/// test translating virtual addresses to physical addresses
/// * `phys_offset` just to see that identity-mapping is used
pub fn testTranslating( paging: &mut HPaging, phys_offset: u64 ) {
    println!( "test address translations" );
    
    let addrs = [
        // identity-mapped so it should be 0x0 as well
        phys_offset,
        0xfff,
        0x1000,
        0xb8000,
        0x7fffff,
        0x800000,
        0xffffffff,
    ];
        
    for i in 0..addrs.len() {
        let testPhys = paging.to_Phys( VirtAddr::new( addrs[i] ) );
        println!( "  virt: {:#x} to {:?}", addrs[i], testPhys );
    }
}