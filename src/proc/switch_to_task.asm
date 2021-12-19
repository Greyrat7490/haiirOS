[BITS 64]

section .text

global jump_usermode
global flush_tss

; GDT_KERNEL_NULL 0x0
; GDT_KERNEL_CODE 0x8
; GDT_KERNEL_DATA 0x10
; GDT_USER_NULL 0x18
; GDT_USER_DATA 0x20
; GDT_USER_CODE 0x28
; GDT_TSS 0x30


flush_tss:
    mov ax, 0x30                        ; requested privilege level is 0 (bits 0 and 1 are 0)
    ltr ax
    ret

; rdi (1st arg) user_stack_top
; rsi (2st arg) user_function
; rdx (3st arg) pml4_addr
jump_usermode:
    cli

    mov rbx, rdx

    mov rcx, 0xc0000080
    rdmsr
    or eax, 1                           ; enable sysret/syscall
    wrmsr
    mov rcx, 0xc0000081                 ; get STAR
    rdmsr
    mov edx, 0x00180008                 ; gdt entry: 0x0018 for sysret, gdt entry: 0x0008 for syscall
    wrmsr                               ; write STAR
                                        ; sysret: cs = 0x18 + 0x10, ss = 0x18 + 0x8
                                        ; syscall: cs = 0x8, ss = 0x8 + 0x8

    mov rcx, rsi                        ; rcx will be loaded into RIP
    mov rsp, rdi                        ; set user stack pointer
    mov cr3, rbx                        ; set cr3 to new pml4 table

    mov r11, 0x200                      ; r11 will be loaded into RFLAGS, 0x200 to enable interrupt
    o64 sysret                          ; o64 to keep in long mode
