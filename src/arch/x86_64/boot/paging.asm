; **********************************************************
;
; Paging information:
; - Using Level 4 Paging
; - PML4-Table -> PDP-Table -> PD-Table -> PT-Table
; - identity mapping
; - Entry size 8 Byte
; - Table size 4KiB
; - Page size 4KiB
;
;                       Paging-Tables information
; |         Table Count           |  Table size / Table Entries  |
;
; |         1x PML4-Table         |   1 * 4KiB  /  1 Entry       |
; |         1x PDP-Table          |   1 * 4KiB  /  1 Entry       |
; |         1x PD-Table           |   1 * 4KiB  /  4 Entries     |
; |         4x PT-Table           |   4 * 4KiB  /  512 Entries   |
;
; |   Memory occupied by Tables   |         Memory mapped        |
; |    4 * 512 * 8Byte = 16KiB    |    512 * 4 * 4KiB = 8MiB     |
;
;
;       Virtual -> Physical
; ------------------------------
; |            0x0             |        - unmapped for testing mapping later
; |            0xfff           |        - ....
; |     0xb8000 -> 0xb8000     |        - VGA-Text-Buffer
; |    0x100000 -> 0x100000    |        - Multiboot_header
; |    0x101000 -> 0x101000    |        - Paging-Tables
; |    0x108000 -> 0x108000    |        - Kernel, Stack, ...
; |    0x7fffff -> 0x7fffff    |
; | -------------------------- |        - 8MiB are mapped
; |          0x800000          |        - unmapped
; |            ....            |        - ....
; |            ....            |        - ....
; ------------------------------
;
; **********************************************************

global enable_paging

section .text

extern PML4_table
extern PDP_table
extern PD_table
extern PT_table

[BITS 32]
enable_paging:
    .prepair:
        ; map first PML4 entry
        mov eax, PDP_table
        or eax, 11b             ; present + writable
        mov [PML4_table], eax

        ; map first PDP entry
        mov eax, PD_table
        or eax, 11b
        mov [PDP_table], eax

        ; map first 4 PD entries
        mov edi, PD_table
        mov ebx, PT_table
        mov ecx, 0              ; counter

        or ebx, 11b             ; present + writable
        .map_PD:
            mov [edi + ecx * 8], ebx
            add ebx, 0x1000     ; next physical address to map (4KiB steps)

            inc ecx

            cmp ecx, 4
            jne .map_PD

        ; map all 4 PT tables
        ; leave 0x0 - 0xfff unmapped just for testing
        mov edi, PT_table
        mov ecx, 1              ; start entry
        mov ebx, 0x1000         ; start physical address
                                ; ecx-th entry has to be ecx-th address
        or ebx, 11b
        .map_PT:
            mov [edi + ecx * 8], ebx
            add ebx, 0x1000     ; next physical address to map (4KiB steps)

            inc ecx

            cmp ecx, 512 * 4    ; 512 Entries * 4 PageTables
            jne .map_PT

    ; update control registery cr0, cr3 to enable paging
    .enable:
        ; load PML4_table to cr3
        mov eax, PML4_table
        mov cr3, eax

        ; enable Physical Address Extension (PAE) with cr4 register
        mov eax, cr4
        or eax, 1 << 5          ; bit 5 is PAE-flag
        mov cr4, eax

        ; enable long mode in the EFER MSR (Model Specific Register)
        mov ecx, 0xc0000080
        rdmsr
        or eax, 1 << 8          ; bit 8 is the long mode bit
        wrmsr

        ; enable paging with the cr0
        mov eax, cr0
        or eax, 1 << 31         ; set bit 31 to enable paging
        mov cr0, eax
    ret
