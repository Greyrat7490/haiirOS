section .text

[BITS 32]
; 512( PML4 entries ) x 512( PDP entries ) x 1GiB pages
enable_paging:
    .prepair:
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

    ; update control registery cr0, cr3 to enable paging
    .enable:
        ; load PML4_table to cr3
        mov eax, PML4_table
        mov cr3, eax

        ; enable paging with the cr0
        mov eax, cr0
        or eax, 1 << 31 ; set bit 31 to enable paging
        mov cr0, eax
    ret

section .bss

align 4096
PML4_table:
    resb 4096
PDP_table:
    resb 4096