# haiirOS

## My first Kernel based on C

> Fun project to learn C, Assembly x86 and how Kernels work

## Support for

* x86_64 little endian
* Legacy boot (enable CSM if you have UEFI)
* no RAID support (disable RAID in BIOS to get AHCI controller instead)

## ToDo-List

* [ ] Documention
  * [ ] VGA text buffer
  * [ ] Paging
  * [ ] mapping
* [ ] drivers
  * [x] AHCI
  * [ ] XHCI
  * [ ] NVMe
* [x] Interrupts
  * [x] Keyboard
  * [x] timer
  * [x] exceptions
* [ ] Memory
  * [x] get memory map
  * [x] create page tables
  * [x] mapping
  * [x] frame allocator (really basic)
    * [x] use memory map
  * [ ] heap allocation
* [x] Error handling
  * [x] exceptions
* [x] Basic IO
  * [x] print
  * [x] keyboard input
* [ ] Basic syscalls
  * [x] write
  * [ ] exit
  * [x] mmap
  * [x] sched_yield
  * [ ] fork
  * [ ] open
  * [ ] read
  * [ ] close
  * [ ] shutdown
  * [ ] restart
  * [ ] kill
  * [ ] getcwd
  * [ ] mkdir
  * [ ] rmdir
  * [ ] rename
  * [ ] pause
  * [ ] nanosleep

---

## Environment

* OS:
  * Linux
  * Windows with WSL2
* IDE:
    * VSCode
    * Neovim

---

## Dev dependencies

* create iso:
  * `sudo apt install grub2 xorriso` (debian based)
  * `sudo pacman -S libisoburn` (arch based)
* VSCode:
  * install extension `C/C++` (ms-vscode.cpptools) (language server / Debugging)
* Neovim:
  * install [cpptools](https://github.com/mfussenegger/nvim-dap/wiki/C-C---Rust-(gdb-via--vscode-cpptools)) (Debugger)
  * install `clangd` (i.e. via lsp_installer) (language server)
* compiling/(assembling):
  * install `nasm` (assembler)
  * install `ld` (Linker)
  * install `gcc` (should already be installed) (C compiler)
  * install `make` (run Makefile / automation)
* Testing/Debugging:
  * install `qemu`/`qemu-desktop` (virtual-machine)

---

## Run kernel in a VM

* run `make run`

## Debugging

* run `make debug`
* VSCode
  * press f5 or run task "(gdb) Launch"
  * using extension: "C/C++" (ms-vscode.cpptools)
* Neovim
  * press f5 or `:lua require'dap'.continue()`
  * using plugin: [nvim-dap](https://github.com/mfussenegger/nvim-dap)

---

## Resources

* [Main source for developing an OS](wiki.osdev.org/Main_Page)
