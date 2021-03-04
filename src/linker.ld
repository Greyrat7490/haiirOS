ENTRY(bootloader_start)

SECTIONS {
	. = 1M;

	.boot : {
		. = ALIGN(8);
		*(.multiboot_header)
		. = ALIGN(4K);

		PML4_table = .; 	/* 0x101000 */
		. += 0x1000;
		PDP_table = .; 		/* 0x102000 */
		. += 0x1000;
		PD_table = .; 		/* 0x103000 */
		. += 0x1000;
		PT_table = .; 		/* 0x104000 */
		. += 512 * 0x1000;
							/* 0x124000 */
	}

	. += 4K;				/* 0x124000 - 0x125000 for IDT */

	.text :	{
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