kernel := build/kernel.bin
iso := build/kernel.iso
os := target/haiirOS-x86_64/debug/libhaiirOS.a

linker_script := src/arch/x86_64/linker.ld
asm_src := src/arch/x86_64/bootloader.asm src/arch/x86_64/multiboot_header.asm
grub_cfg := src/arch/x86_64/grub.cfg

asm_obj := build/obj/bootloader.o build/obj/multiboot_header.o


all: $(kernel)

clean:
	cargo clean
	rm -rf build

iso: $(iso)

run: $(iso)
	qemu-system-x86_64 -hda $(iso)

$(iso): $(kernel) $(grub_cfg)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	grub-mkrescue -o $(iso) build/isofiles

$(kernel): cargo $(asm_obj) $(linker_script)
	ld -n -T $(linker_script) -o $(kernel) $(asm_obj) $(os)

cargo:
	cargo build

build/obj/%.o: src/arch/x86_64/%.asm
	mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@