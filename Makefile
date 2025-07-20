CC=i686-elf-gcc
LD=i686-elf-ld
AS=nasm
CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS=-T linker.ld
ISO=terminal.iso
OB=objcopy

all: kernel.bin $(ISO)

# Compile objects
boot.o: src/boot.s
	$(AS) -f elf32 $< -o $@

kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

snake.o: src/apps/snake.c
	$(CC) $(CFLAGS) -c $< -o $@

calc.o: src/apps/calc.c
	$(CC) $(CFLAGS) -c $< -o $@

notepad.o: src/apps/notepad.c
	$(CC) $(CFLAGS) -c $< -o $@

fs.o: src/fs.c
	$(CC) $(CFLAGS) -c $< -o $@

diskio.o: src/disk/diskio.c
	$(CC) $(CFLAGS) -c $< -o $@

installer.o: src/installer.c
	$(CC) $(CFLAGS) -c $< -o $@

io.o: src/net/io.c
	$(CC) $(CFLAGS) -c $< -o $@

wifi.o: src/net/wifi.c
	$(CC) $(CFLAGS) -c $< -o $@

print.o: src/print.c
	$(CC) $(CFLAGS) -c $< -o $@

mouse.o: src/graphics/mouse.c
	$(CC) $(CFLAGS) -c $< -o $@

window.o: src/graphics/window.c
	$(CC) $(CFLAGS) -c $< -o $@

framebuffer.o: src/graphics/framebuffer.c
	$(CC) $(CFLAGS) -c $< -o $@

gui.o: src/graphics/gui.c
	$(CC) $(CFLAGS) -c $< -o $@

ps2mouse.o: src/graphics/ps2mouse.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel without installer (optional, for pure kernel.bin)
kernel.bin: boot.o kernel.o snake.o calc.o notepad.o fs.o io.o print.o wifi.o framebuffer.o gui.o ps2mouse.o diskio.o installer.o
	$(LD) $(LDFLAGS) -o kernel.bin boot.o kernel.o snake.o calc.o notepad.o fs.o io.o print.o wifi.o framebuffer.o gui.o ps2mouse.o diskio.o installer.o

# Generate blob object
kernel_blob.o: kernel.bin
	objcopy -I binary -O elf32-i386 -B i386 kernel.bin kernel_blob.o

# Link kernel_installer.bin with blob object
kernel_installer.bin: boot.o kernel.o snake.o calc.o notepad.o fs.o io.o print.o wifi.o framebuffer.o gui.o ps2mouse.o diskio.o installer.o kernel_blob.o
	$(LD) $(LDFLAGS) -o kernel_installer.bin boot.o kernel.o snake.o calc.o notepad.o fs.o io.o print.o wifi.o framebuffer.o gui.o ps2mouse.o diskio.o installer.o kernel_blob.o

# ISO generation uses kernel.bin by default. If you want ISO to use the installer, change it to installer.bin
$(ISO): kernel.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

clean:
	rm -rf *.o *.bin isodir $(ISO)

.PHONY: all clean