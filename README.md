# haiirOS

## My first Kernel based on C

> Fun project to learn C, Assambly x86 and how Kernels work

## Support for

* x86_64
* Legacy boot ( so be sure CSM is working if you have UEFI )

## Debugging

* press f5 in VS Code or run "(gdb) Launch"
* using vscode extension: "C/C++" ( ms-vscode.cpptools )

## Environment

* using WSL2 / Linux
* qemu-system-x86_64 as virtual-machine
* make for automation
* grub-mkrescue to create grub2 multiboot iso ( "sudo apt install grub2 xorriso" if needed )
* ld as linker
* gcc as c compiler
* VS Code as Editor( IDE )
* Extensions:
  * IntelliSense / Debugging: C/C++ ( ms-vscode.cpptools )

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
