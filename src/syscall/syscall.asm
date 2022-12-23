[BITS 64]

section .text

global jump_usermode
global enable_syscalls

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

    o64 sysret                          ; o64 to keep in long mode

syscall_entry:
    ; save important registers
    push rcx
    push r11

    mov rbx, rsp        ; save user stack
    mov rsp, tmp_stack
    sti

    extern syscall_table
    mov rax, [syscall_table + rax * 8]
    call rax

    ; go back (in usermode)
    cli
    mov rsp, rbx
    pop r11
    pop rcx
    mov r11, (1 << 9)   ; enable interrupts
    o64 sysret


; temporary (kernel)stack for syscalls
section .bss
align 4096
    resb 4096
tmp_stack:
