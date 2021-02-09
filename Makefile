project_name := haiirOS
kernel := build/$(project_name).bin
iso := build/$(project_name).iso

linker_script := src/linker.ld
asm_src := src/arch/x86_64/boot/bootloader.asm src/arch/x86_64/boot/multiboot_header.asm
grub_cfg := src/arch/x86_64/boot/grub.cfg

rust_obj := target/$(project_name)-x86_64/debug/lib$(project_name).a
asm_obj := $(patsubst src/arch/x86_64/boot/%.asm, build/obj/%.o, $(asm_src))


all: $(kernel)

clean:
	cargo clean
	rm -rf build

iso: $(iso)

run: $(iso)
	qemu-system-x86_64 -hda $(iso)

$(iso): $(kernel) $(grub_cfg)
	mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/$(project_name).bin
	cp $(grub_cfg) build/isofiles/boot/grub
	grub-mkrescue -o $(iso) build/isofiles

$(kernel): cargo $(asm_obj) $(linker_script)
	ld -n -T $(linker_script) -o $(kernel) $(asm_obj) $(rust_obj)

cargo:
	cargo build

build/obj/%.o: src/arch/x86_64/boot/%.asm
	mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@