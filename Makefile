project_name := haiirOS

debug_kernel := build/debug/$(project_name).bin
release_kernel := build/release/$(project_name).bin
debug_iso := build/debug/$(project_name).iso
release_iso := build/release/$(project_name).iso

asm_src := $(shell find src -name "*.asm")
c_src := $(shell find src -name "*.c")
grub_cfg := src/arch/x86_64/boot/grub.cfg

c_debug_obj := $(patsubst src/%.c, build/debug/obj/c/%.o, $(c_src))
c_release_obj := $(patsubst src/%.c, build/release/obj/c/%.o, $(c_src))

asm_debug_obj := $(patsubst src/%.asm, build/debug/obj/asm/%.o, $(asm_src))
asm_release_obj := $(patsubst src/%.asm, build/release/obj/asm/%.o, $(asm_src))

CFLAGS := -MD -ffreestanding -fno-stack-protector -z max-page-size=0x1000 -mgeneral-regs-only
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
CFLAGS += -Wall -Wextra -pedantic -nostdlib -Isrc -std=c11

LDFLAGS := -m elf_x86_64 -nostdlib


.PHONY: all clean run release debug build-debug

all: build-debug

clean:
	rm -rf build

release: CFLAGS += -O3
release: $(release_iso)

build-debug: CFLAGS += -ggdb -Og -D DEBUG
build-debug: $(debug_iso)

run: release
	qemu-system-x86_64 -drive format=raw,file=$(release_iso)

debug: build-debug
	qemu-system-x86_64 -s -S -drive format=raw,file=$(debug_iso)

# debug --------------------------------------------------
$(debug_iso): $(debug_kernel)
	mkdir -p build/debug/iso/boot/grub
	cp $(debug_kernel) build/debug/iso/boot/$(project_name).bin
	grub-mkrescue -o $(debug_iso) build/debug/iso

$(debug_kernel): $(asm_debug_obj) $(c_debug_obj)
	ld $(asm_debug_obj) $(c_debug_obj) -o $(debug_kernel) $(LDFLAGS)

build/debug/obj/asm/%.o: src/%.asm
	mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

build/debug/obj/c/%.o: src/%.c
	mkdir -p $(shell dirname $@)
	gcc -c $< -o $@ $(CFLAGS)

# release --------------------------------------------------
$(release_iso): $(release_kernel)
	mkdir -p build/release/iso/boot/grub
	cp $(release_kernel) build/release/iso/boot/$(project_name).bin
	grub-mkrescue -o $(release_iso) build/release/iso

$(release_kernel): $(asm_release_obj) $(c_release_obj)
	ld $(asm_release_obj) $(c_release_obj) -o $(release_kernel) $(LDFLAGS)

build/release/obj/asm/%.o: src/%.asm
	mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

build/release/obj/c/%.o: src/%.c
	mkdir -p $(shell dirname $@)
	gcc -c $< -o $@ $(CFLAGS)


-include $(c_debug_obj:.o=.d)
-include $(c_release_obj:.o=.d)
