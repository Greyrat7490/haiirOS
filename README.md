# haiirOS

## My first Kernel based on Rust

> Fun project to learn Rust, Assambly x86 and how Kernels work

## Support for

* x86_64
* Legacy boot ( so be sure CSM is working if you have UEFI )

## Debugging

* press f5 in VSCode or run "gdb remote debug"
* breakpoints are buggy ( breaks sometimes in memcpy )

## ToDo-List

* Documention
  * VGA text buffer
  * Paging
* Interrupts
  * Keyboard
* Memory
  * memory maps
  * page table
  * mapping
  * frame allocator
  * heap allocation
* Error handling
  * panic
  * bootloader
  * exceptions
* Basic IO
  * print
  * keyboard input
* Basic syscalls

---

* Maybe support for ARM
* Maybe support for 32bit

## Resources

* [Main source for developing an OS](wiki.osdev.org/Main_Page)
* [Also a great page](os.phil-opp.com/multiboot-kernel)
