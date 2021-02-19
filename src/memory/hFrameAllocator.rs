use x86_64::structures::paging::{ FrameAllocator, Size4KiB };
use x86_64::structures::paging::PhysFrame;

pub struct MemoryAreaAllocator;

unsafe impl FrameAllocator<Size4KiB> for MemoryAreaAllocator {
    fn allocate_frame(&mut self) -> Option<PhysFrame<Size4KiB>> {
        None
    }
}