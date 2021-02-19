#![allow(dead_code)]

use crate::printf;
use crate::println;

use x86_64::{ PhysAddr, VirtAddr };
use x86_64::structures::paging::{
    Translate,
    PageTable,
    OffsetPageTable
};

pub struct HPaging {
    offset_page_table: OffsetPageTable<'static>,
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

    println!( "PML4 table addr:\n  phys: {:#x}\n  virt:{:#x}", phys_addr, virt_addr );

    let tablePtr = virt_addr as *mut PageTable;

    &mut *tablePtr
}