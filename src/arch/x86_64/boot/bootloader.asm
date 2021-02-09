global bootloader_start

; nasm searchs in the current directory it is running 
%include "src/arch/x86_64/boot/gdt64.asm"
%include "src/arch/x86_64/boot/paging.asm"
%include "src/arch/x86_64/boot/long_mode.asm"

section .text
[BITS 32]
bootloader_start:
    mov esp, stack_top

    call enable_lm
    call enable_paging
	call load_gdt64

    ; far jump to switch to long mode
    jmp gdt64.code:long_mode_entry

    hlt

section .bss

stack_bottom:
    resb 64
stack_top: