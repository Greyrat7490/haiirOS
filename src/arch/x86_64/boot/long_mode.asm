section .text

[BITS 32]
enable_lm:
    ; enable Physical Address Extension( PAE ) with cr4 register
    mov eax, cr4
    or eax, 1 << 5 ; bit 5 is PAE-flag
    mov cr4, eax

    ; enable long mode in the EFER MSR( Model Specific Register )
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8 ; bit 8 is the long mode bit
    wrmsr
    ret

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
    mov edi, [multiboot_info_ptr] ; first arg of kernel_main
    extern kernel_main
    call kernel_main
    ; -----------------------------------------

    hlt