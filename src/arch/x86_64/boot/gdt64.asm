section .text

[BITS 32]
load_gdt64:
    lgdt [gdt64.pointer]
    ret

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