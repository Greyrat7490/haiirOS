global bios_service_wrapped
extern bios_service
extern gdt64.code
extern gdt64.code32
extern gdt64.data32
extern gdt64.data
extern gdt64.pointer

[BITS 64]
bios_service_wrapped:
    push gdt64.code32
    mov ecx, .comp_mode
    push rcx
    retfq
[BITS 32]
.comp_mode:
    cli
    mov ax, gdt64.data32
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, cr0
    and eax, ~(1 << 31)     ; disable paging
    mov cr0, eax

    mov ecx, 0xc0000080
    rdmsr
    and eax, ~(1 << 8)      ; disable long mode
    wrmsr

    call [bios_service]

    mov ecx, 0xc0000080
    rdmsr
    or eax, 1 << 8          ; enable long mode
    wrmsr

    mov eax, cr0
    or eax, 1 << 31         ; enable paging
    mov cr0, eax

    lgdt [gdt64.pointer]
    jmp gdt64.code:.init64
[BITS 64]
.init64:
    mov ax, gdt64.data
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
