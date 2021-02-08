global bootloader_start

section .text

[BITS 32]
bootloader_start:
  	mov esp, stack_top 

	call setup_paging
    call enable_lm

	; load 64bit GDT
    lgdt [gdt64.pointer]

    ; far jump to switch to long mode
    jmp gdt64.code:long_mode_entry

    hlt


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

    ; update control registery cr0, cr3 to enable paging
    .enable_paging:
        ; load PML4_table to cr3
        mov eax, PML4_table
        mov cr3, eax
        
        ; enable paging with the cr0
        mov eax, cr0
        or eax, 1 << 31 ; set bit 31 to enable paging
        mov cr0, eax

        ret

; 512( PML4 entries ) x 512( PDP entries ) x 1GiB pages
setup_paging:
    .map_PML4:
        mov eax, PDP_table
        or eax, 11b ; present + writable
        mov [PML4_table], eax

    mov edi, PDP_table
    mov ebx, 10000011b ; huge + present + writable
    mov ecx, 512 ; loop 512 times ( 512 entries )
    .map_PDP:
        mov [edi], ebx
        add ebx, 0x40000000 ; 1GiB
        add edi, 8 ; next entry ( every entry is 8 bit )

        loop .map_PDP
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
    extern kernel_main
    call kernel_main
    ; -----------------------------------------

    hlt


section .bss
; all paging tables ----------------------------
align 4096
PML4_table:
    resb 4096
PDP_table:
    resb 4096
; -----------------------------------------------------

; stack -----------------------------------------------
stack_bottom:
    resb 64
stack_top:
; -----------------------------------------------------

section .rodata
; global descriptor table ( 64bit ) -------------------
gdt64:
.null: equ $ - gdt64    ; null descriptor
    dq 0 
.code: equ $ - gdt64    ; code descriptor
    dw 0                ; limit ( low )
    dw 0                ; base ( low )
    db 0                ; base ( middle )
    db 10011101b        ; access ( execute/read )
    db 10101111b        ; Granularity, 64 bits flag, limit19:16
    db 0                ; base ( high )
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
; -----------------------------------------------------