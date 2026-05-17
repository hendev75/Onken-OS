CC = gcc
LD = ld
AS = nasm

CFLAGS = -m64 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-stack-check -mno-red-zone -mno-sse -mno-mmx -mno-avx -fno-builtin -U_FORTIFY_SOURCE
ASFLAGS = -f elf64
LDFLAGS = -m elf_x86_64 -nostdlib -static -z text -z max-page-size=0x1000 -T linker.ld

OBJ = boot/boot.o \
      kernel/kernel.o \
      kernel/string.o \
      kernel/mm.o \
      kernel/fs.o \
      drivers/ps2.o \
      gui/fb.o \
      gui/window.o \
      shell/shell.o \
      kernel/interrupts/idt.o \
      kernel/interrupts/interrupts.o \
      kernel/time/pit.o \
      kernel/scheduler/scheduler.o \
      kernel/tasking/app.o \
      kernel/syscalls/syscall.o \
      apps/htop/htop.o \
      apps/yano/yano.o \
      apps/sysinfo/sysinfo.o \
      apps/browser/browser.o \
      apps/files/files.o \
      apps/settings/settings.o \
      apps/terminal/terminal.o \
      drivers/sound.o \
      drivers/rtc.o \
      drivers/pci.o \
      drivers/cpuid.o \
      gui/stb_image.o \
      apps/player/player.o \
      apps/imageviewer/imageviewer.o
TARGET = oneko.bin
ISO = oneko.iso

all: limine_bin $(ISO)

limine_bin: reference/BoredOS-26.6.0-RC1/limine/limine.c
	$(CC) -O2 $< -o $@

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(TARGET)
	mkdir -p isodir
	cp $(TARGET) isodir/
	cp limine.conf isodir/
	cp limine/limine-bios.sys isodir/
	cp limine/limine-bios-cd.bin isodir/
	cp limine/limine-uefi-cd.bin isodir/
	mkdir -p isodir/EFI/BOOT
	cp limine/BOOTX64.EFI isodir/EFI/BOOT/
	cp limine/BOOTIA32.EFI isodir/EFI/BOOT/
	xorriso -as mkisofs -R -J -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		isodir -o $(ISO)
	./limine_bin bios-install $(ISO)
	rm -rf isodir

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(ISO)
	rm -rf isodir

run: all
	qemu-system-x86_64 -m 512 -cdrom $(ISO) -vga std -serial stdio -global VGA.xres=1280 -global VGA.yres=720