CC = gcc
CFLAGS = -O2 \
			-Wall \
			-Wextra \
			-fpic \
			-ffreestanding \
			-fno-stack-protector \
			-fno-stack-check \
			-fshort-wchar \
			-mno-red-zone \
			-maccumulate-outgoing-args

LD = ld
LDFLAGS = -shared \
			 -Bsymbolic \
			 -L/usr/lib \
			 -L/usr/lib/gnuefi \
			 -T/usr/lib/elf_x86_64_efi.lds \
			 /usr/lib/crt0-efi-x86_64.o
LDLIBS = -lgnuefi -lefi

.PHONY: all run debug format
all: disk.img tags boot.so.debug

run: disk.img
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF.4m.fd \
		-drive format=raw,file=$< \
		-device virtio-rng-pci \
		-m 1G

debug: disk.img
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF.4m.fd \
		-drive format=raw,file=$< \
		-device virtio-rng-pci \
		-m 1G -s -S

format:
	find src/ -name *.c -o -name *.h -exec clang-format --verbose -i {} \;

tags: $(wildcard src/**/*)
	ctags src/*

disk.img: src/boot.efi
	dd if=/dev/zero of=$@ bs=1M count=65
	sfdisk $@ < disk_layout.txt
	mkfs.fat -F 32 --offset 2048 $@ 65536
	mmd -i $@@@1M ::/EFI
	mmd -i $@@@1M ::/EFI/BOOT
	mcopy -i $@@@1M $< ::/EFI/BOOT/BOOTX64.EFI

src/boot.efi: src/boot.so
	objcopy \
		-j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
		--output-target efi-app-x86_64 \
		--subsystem=10 \
		$< $@

boot.so.debug: src/boot.so
	objcopy --only-keep-debug $^ $@

src/boot.so: src/util.o src/boot.o src/graphics.o src/debug.o src/random.o src/input.o src/time.o src/log.o src/chip8.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LDLIBS)

src/boot.o: src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/util.o src/graphics.o src/debug.o src/random.o src/input.o src/time.o src/log.o src/chip8.o: src/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@
