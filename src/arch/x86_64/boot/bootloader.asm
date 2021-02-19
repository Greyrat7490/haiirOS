global bootloader_start

extern enable_paging
extern long_mode_entry

section .text
[BITS 32]
bootloader_start:
    mov esp, stack_top
	push ebx

    call enable_paging
	lgdt [gdt64.pointer]
    
	pop ebx
    ; far jump to switch to long mode
    jmp gdt64.code:long_mode_entry

    hlt

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

section .bss

align 4096
stack_bottom:
    resb 4096 * 16
stack_top: