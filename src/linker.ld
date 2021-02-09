ENTRY(bootloader_start)

SECTIONS {
    . = 1M;

	.boot :	{
		*(.multiboot_header)
	}

	.text :	{
		*(.text)
	}

	.bss : {
		*(.bss)
	}

	.rodata : {
		*(.rodata)
	}
}