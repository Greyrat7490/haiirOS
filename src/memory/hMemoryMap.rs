#![allow(dead_code)]

use core::borrow::Borrow;

use multiboot2::{ BootInformation, ElfSectionsTag, MemoryMapTag };

use crate::println;
use crate::printf;

pub struct HBootInfo {
    pub boot_info: BootInformation
}

impl HBootInfo {
    pub fn new( multiboot_info_ptr: usize ) -> Self {
        HBootInfo { 
            boot_info: unsafe{ multiboot2::load( multiboot_info_ptr ) }
        }
    }

    pub fn printAddr( &self ) {
        println!( "Multiboot info addr: {:#x}", self.boot_info.start_address() );
    }
}

pub struct HMemoryInfo<'a> {
    pub phys_offset: u64,
    pub memory_map: &'a MemoryMapTag,
    efi_sections: ElfSectionsTag,

    pub unmapped_frame_start: u64,

    kernel_addr: usize,
    kernel_end: usize,
    multiboot_addr: usize,
    multiboot_end: usize
}

fn get_memory_map<'a>( boot_info: &'a BootInformation ) -> &'a MemoryMapTag {
        
    let mem_map_tag = boot_info.memory_map_tag()
        .expect( "memory map tag is required" );

    mem_map_tag
}

fn get_efi_sections( boot_info: &BootInformation ) -> ElfSectionsTag {
    boot_info.elf_sections_tag()
        .expect( "Elf-sections tag is required" )
}

impl<'a> HMemoryInfo<'a> {
    pub fn new( boot_info: &'a HBootInfo ) -> Self {
        let memory_map: &'a MemoryMapTag = get_memory_map( boot_info.boot_info.borrow() );
        let efi_sections = get_efi_sections( boot_info.boot_info.borrow() );
        
        let kernel_addr = efi_sections.sections()
            .map( |s| s.start_address() ).min().unwrap() as usize;
        let kernel_end = efi_sections.sections()
            .map( |s| s.end_address() ).max().unwrap() as usize;

        let multiboot_addr = boot_info.boot_info.start_address();
        let multiboot_end = boot_info.boot_info.end_address();

        let phys_offset = memory_map.all_memory_areas().nth(0).unwrap()
            .start_address();

        Self { 
            phys_offset,
            memory_map,
            efi_sections,
            unmapped_frame_start: 0x800000,
            kernel_addr,
            kernel_end,
            multiboot_addr,
            multiboot_end
        }
    }

    pub fn print( &self ) {
        println!( "available memory areas:" );
        for area in self.memory_map.memory_areas() {
            println!( "  from {:#x} to {:#x}", area.start_address(), area.end_address() );
        }

        println!( "kernel:\n  from {:#x} to {:#x}", self.kernel_addr, self.kernel_end );
        println!( "multiboot:\n  from {:#x} to {:#x}", self.multiboot_addr, self.multiboot_end );    
    }
}
