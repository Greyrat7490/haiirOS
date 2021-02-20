# haiirOS

## My first Kernel based on Rust

> Fun project to learn Rust, Assambly x86 and how Kernels work

## Support for

* x86_64
* Legacy boot ( so be sure CSM is working if you have UEFI )

## Debugging

* press f5 in VS Code or run "Launch(remote)"
* using vscode extension: "GDB Debugger - Beyond"

## Environment

* using WSL2 / Linux
* qemu-system-x86_64 as virtual-machine
* make for automation
* grub-mkrescue to create grub2 multiboot iso
* ld as linker
* Rust nightly
* VS Code as Editor( IDE )
* Extensions:
  * Debugging: GDB Debugger - Beyond ( coolchyni.beyond-debug )
  * IntelliSense: rust-analyzer ( matklad.rust-analyzer )
  * TOML: Better TOML ( bungcip.better-toml )
  * Crates: crates ( serayuzgur.crates )

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
