ENTRY(bootloader_start)

SECTIONS {
    . = 1M;

    .boot : {
        . = ALIGN(8);
        *(.multiboot_header)
        . = ALIGN(4K);

        PML4_table = .;     /* 0x101000 */
        . += 8;
        . = ALIGN(4K);
        PDP_table = .;      /* 0x102000 */
        . += 8;
        . = ALIGN(4K);
        PD_table = .;       /* 0x103000 */
        . += 4 * 8;
        . = ALIGN(4K);
        PT_table = .;       /* 0x104000 */
        . += 4 * 512 * 8;
        . = ALIGN(4K);
                            /* 0x108000 */
    }

    . += 32K;               /* 0x108000 - 0x110000 for IDT */

    .text : {
        *(.text .text.*)
        . = ALIGN(4K);
    }
    
    .bss : {
        *(.bss)
        . = ALIGN(4K);
    }

    .rodata : {
        *(.rodata .rodata.*)
        . = ALIGN(4K);
    }

    .data : {
        *(.data .data.*)
        . = ALIGN(4K);
    }

    .data.rel.ro : {
        *(.data.rel.ro.local*) *(.data.rel.ro .data.rel.ro.*)
        . = ALIGN(4K);
    }
}
