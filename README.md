# haiirOS

## My first Kernel based on C

> Fun project to learn C, Assambly x86 and how Kernels work

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
* gcc as c compiler
* VS Code as Editor( IDE )
* Extensions:
  * Debugging: GDB Debugger - Beyond ( coolchyni.beyond-debug )
  * IntelliSense: C/C++ ( ms-vscode.cpptools )

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
