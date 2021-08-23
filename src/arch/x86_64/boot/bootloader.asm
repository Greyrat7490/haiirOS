global bootloader_start

extern enable_paging
extern long_mode_entry

section .text
[BITS 32]
bootloader_start:
    cli

    mov esp, stack_top
    push ebx

    call enable_paging
    lgdt [gdt64.pointer]
    
    pop ebx
    ; far jump to switch to long mode
    jmp gdt64.code:long_mode_entry

    hlt


[BITS 64]
global load_tss
load_tss:
    ; Load TSS descriptor into GDT ------------
    mov rdi, gdt64
    add rdi, gdt64.tss
    mov rax, TSS64
    mov word [rdi + 2], ax      ; set base( 15 - 0 )
    shr rax, 16
    mov byte [rdi + 4], al      ; set base( 23 - 16 )
    shr rax, 8
    mov byte [rdi + 7], al      ; set base( 31 - 24 )
    shr rax, 8
    mov dword [rdi + 8], eax    ; set base( 63 - 32 )
    ; -----------------------------------------

    ; reload gdt
    lgdt [gdt64.pointer]
    ret


section .rodata

global TSS64
global IST

align 16
TSS64:
    dd 0            ; reserved
    times 3 dq 0    ; RSP 0 - 2
    dq 0            ; reserved
IST:
    dq ist_stack1   ; IST 1, no maskable interrupt
    dq ist_stack2   ; IST 2, double fault
    dq 0            ; IST 3
    dq 0            ; IST 4
    dq 0            ; IST 5
    dq 0            ; IST 6
    dq 0            ; IST 7
    dq 0            ; reserved
    dw 0            ; reserved
    dw 0            ; IO-map addr
TSS_SIZE: equ $ - TSS64 - 1
    
global tss_pointer
tss_pointer: dq TSS64

global kernel_stack
kernel_stack: dq stack_top

global usr_stack
usr_stack: dq test_stack_top

; global descriptor table ( 64bit ) -------------------
align 16
global gdt64
gdt64:
.null: equ $ - gdt64        ; kernel null descriptor
    dq 0
.code: equ $ - gdt64        ; ring 0 code descriptor
    dw 0                    ; limit ( low )
    dw 0                    ; base ( low )
    db 0                    ; base ( middle )
    db 10011010b            ; present, kernel_mode, code_data_seg, code_seg, executable, read/write
    db 10100000b            ; 4k-paging, long_mode
    db 0                    ; base ( high )
.data: equ $ - gdt64        ; ring 0 data descriptor
    dw 0                    ; limit ( low )
    dw 0                    ; base ( low )
    db 0                    ; base ( middle )
    db 10010010b            ; present, kernel_mode, code_data_seg, data_seg, executable, read/write
    db 10100000b            ; 4k-paging, long_mode
    db 0                    ; base ( high )
.user_null: equ $ - gdt64   ; user null descriptor
    dq 0
.user_data equ $ - gdt64    ; ring 3 data descriptor
    dw 0                    ; limit ( low )
    dw 0                    ; base ( low )
    db 0                    ; base ( middle )
    db 11110010b            ; present, user_mode, code_data_seg, data_seg, executable, read/write
    db 10100000b            ; 4k-paging
    db 0                    ; base ( high )
.user_code equ $ - gdt64    ; ring 3 code descriptor
    dw 0                    ; limit ( low )
    dw 0                    ; base ( low )
    db 0                    ; base ( middle )
    db 11111010b            ; present, user_mode, code_data_seg, code_seg, executable, read/write
    db 10100000b            ; 4k-paging, long_mode
    db 0                    ; base ( high )
.tss: equ $ - gdt64
    dw TSS_SIZE & 0xffff    ; limit( 15 - 0 )
    dw 0                    ; base ( 15 - 0 )
    db 0                    ; base ( 23 - 16 )
    db 10001001b            ; present, kernel_mode, system_seg,
    db 10100000b            ; 4k-paging, long_mode, limit( 19 - 16 )
    db 0                    ; base ( 31 - 24 )
    dd 0                    ; base ( 63 - 32 )
    dd 0                    ; zero
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
; -----------------------------------------------------

section .bss

align 4096
stack_bottom:
    resb 4096 * 6
stack_top:

    resb 4096
ist_stack1:
    resb 4096
ist_stack2:


global test_stack_top
test_stack_bottom:
    resb 4096 * 6
test_stack_top:
    resb 4096 * 6
