global long_mode_entry

section .text

[BITS 64]
long_mode_entry:
    ; set all segment registers to 0 ----------
    xor ax, ax   
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; -----------------------------------------

    ; go to kernel ----------------------------
    mov edi, ebx ; first arg of kernel_main
    extern kernel_main
    call kernel_main
    ; -----------------------------------------

    hlt