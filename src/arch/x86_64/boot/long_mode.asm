global long_mode_entry

section .text

[BITS 64]
long_mode_entry:
    ; set kernel data segment -----------------
    mov ax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; -----------------------------------------

    extern load_tss
    call load_tss

    ; go to kernel ----------------------------
    mov edi, ebx ; first arg of kernel_main
    extern kernel_main
    call kernel_main
    ; -----------------------------------------

    hlt ; will never be called
