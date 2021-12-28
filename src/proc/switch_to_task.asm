[BITS 64]

section .text

global jump_usermode
global enable_syscalls
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

enable_syscalls:
    cli

    mov rcx, 0xc0000080
    rdmsr
    or eax, 1                           ; enable sysret/syscall
    wrmsr

    mov rcx, 0xc0000081                 ; get STAR (set gdt entries for sysret/syscall)
    rdmsr
    mov edx, 0x00180008                 ; gdt entry: 0x0018 for sysret, gdt entry: 0x0008 for syscall
    wrmsr                               ; sysret: cs = 0x18 + 0x10, ss = 0x18 + 0x8
                                        ; syscall: cs = 0x8, ss = 0x8 + 0x8

    mov rcx, 0xc0000082                 ; get LSTAR (target instruction pointer when syscall)
	rdmsr
    mov rdi, syscall_entry
    mov eax, edi                        ; eax = lower 32bits to write into LSTAR
    shr rdi, 32
    mov edx, edi                        ; edx = higher 32bits
    wrmsr

    mov rcx, 0xc0000084                 ; get FSTAR (flags which will be cleared on syscall)
    rdmsr
    or eax, (1 << 9)                    ; disable interrupts
    wrmsr

    sti
    ret


; rdi (1st arg) user_stack_top
; rsi (2st arg) user_function
; rdx (3st arg) pml4_addr
jump_usermode:
    cli

    mov rcx, rsi                        ; rcx will be loaded into RIP
    mov rsp, rdi                        ; set user stack pointer
    mov r11, (1 << 9)                   ; r11 will be loaded into RFLAGS, 9th bit to enable interrupts
    mov cr3, rdx                        ; set cr3 to new pml4 table

    extern switch_task
    mov rax, switch_task                ; to virtual address
    and rax, 0xfff
    mov rbx, 0x10000000000
    add rax, rbx
    push rax                            ; return address if a task ends (switch to next task)

    o64 sysret                          ; o64 to keep in long mode

syscall_entry:
    cli
    mov rbx, rsp                        ; save user stack
    mov rsp, tmp_stack
    sti
 
    push rax

    extern syscall_table
    mov rax, [syscall_table + rax * 8]  ; look into syscall table
    call rax                            ; call the right syscall function

    pop rax

    ; go back in usermode
    mov rsp, rbx
    mov r11, (1 << 9)                   ; enable interrupts
    o64 sysret 
 

section .bss

; temporary (kernel)stack for syscalls
align 4096
    resb 4096
tmp_stack:
