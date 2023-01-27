project_name := haiirOS

debug_kernel := build/debug/$(project_name).bin
release_kernel := build/release/$(project_name).bin
debug_img := build/debug/$(project_name).img
release_img := build/release/$(project_name).img

bloader_img := build/bloader.img

asm_src := $(shell find src -name "*.asm")
c_src := $(shell find src -name "*.c")

c_debug_obj := $(patsubst src/%.c, build/debug/obj/c/%.o, $(c_src))
c_release_obj := $(patsubst src/%.c, build/release/obj/c/%.o, $(c_src))

asm_debug_obj := $(patsubst src/%.asm, build/debug/obj/asm/%.o, $(asm_src))
asm_release_obj := $(patsubst src/%.asm, build/release/obj/asm/%.o, $(asm_src))

CFLAGS := -MD -ffreestanding -fno-stack-protector -z max-page-size=0x1000 -mgeneral-regs-only
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
CFLAGS += -Wall -Wextra -pedantic -nostdlib -Isrc -std=c11

LDFLAGS := -m elf_x86_64 -nostdlib


.PHONY: all clean run release debug build-debug monitor

all: build-debug

clean:
	rm -rf build

release: CFLAGS += -O3
release: $(release_img)

build-debug: CFLAGS += -ggdb -Og -D DEBUG
build-debug: $(debug_img)

run: release
	qemu-system-x86_64 -M q35 -drive format=raw,file=$(release_img)

debug: build-debug
	qemu-system-x86_64 -M q35 -s -S -drive format=raw,file=$(debug_img)

monitor: build-debug
	qemu-system-x86_64 -M q35 -drive format=raw,file=$(debug_img) -no-reboot -no-shutdown -monitor stdio


$(bloader_img):
	mkdir build -p
	cd bloader && make && cp build/bloader.img ../$(bloader_img) && cd ..

# debug --------------------------------------------------
$(debug_img): $(bloader_img) $(debug_kernel)
	cp $(bloader_img) $(debug_img)

	# mount img without root permissions
	tmp_loop=$$(udisksctl loop-setup -f $(debug_img) | awk '{gsub(/.$$/,""); print $$NF}') && \
	tmp_mnt=$$(udisksctl mount -b $$tmp_loop | awk '{print $$NF}') && \
	cp $(debug_kernel) $$tmp_mnt/KERNEL && \
	udisksctl unmount -b $$tmp_loop	&& \
	udisksctl loop-delete -b $$tmp_loop

$(debug_kernel): $(asm_debug_obj) $(c_debug_obj)
	ld $(asm_debug_obj) $(c_debug_obj) -o $(debug_kernel) $(LDFLAGS)

build/debug/obj/asm/%.o: src/%.asm
	mkdir -p $(shell dirname $@)
	nasm -f elf64 $< -o $@

build/debug/obj/c/%.o: src/%.c
	mkdir -p $(shell dirname $@)
	gcc -c $< -o $@ $(CFLAGS)

# release --------------------------------------------------
$(release_img): $(bloader_img) $(release_kernel)
	cp $(bloader_img) $(release_img)

	# mount img without root permissions
	tmp_loop=$$(udisksctl loop-setup -f $(release_img) | awk '{gsub(/.$$/,""); print $$NF}') && \
	tmp_mnt=$$(udisksctl mount -b $$tmp_loop | awk '{print $$NF}') && \
	cp $(release_kernel) $$tmp_mnt/KERNEL && \
	udisksctl unmount -b $$tmp_loop	&& \
	udisksctl loop-delete -b $$tmp_loop

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
