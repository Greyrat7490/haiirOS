[BITS 64]

section .text

global flush_tss

; GDT_TSS = 0x30
flush_tss:
    mov ax, 0x30        ; requested privilege level is 0 (bits 0 and 1 are 0)
    ltr ax
    ret
