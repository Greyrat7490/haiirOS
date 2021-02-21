use x86_64::structures::paging::{ FrameAllocator, Size4KiB };
use x86_64::structures::paging::PhysFrame;

/// Contains all usable Frames from the memory map given from the bootloader
pub struct MemoryMapFrames;

unsafe impl FrameAllocator<Size4KiB> for MemoryMapFrames {
    fn allocate_frame(&mut self) -> Option<PhysFrame<Size4KiB>> {
        None
    }
}