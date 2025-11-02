# Horizon Makefile
# (c) 2025- HorizonOS Project
#
# Building Horizon requires the following tools;
#
# clang --> 18.1.3
# make  --> 4.3
# NASM  --> 2.16.01

# Tools
CC 		:= clang
LD 		:= ld.lld
AS 		:= nasm
OBJC 	:= llvm-objcopy

# Paths
BUILD 	:= build
ISO 	:= $(BUILD)/Horizon.iso
KERNEL 	:= $(BUILD)/kernel.elf

BOOT_SRC := src/boot/boot.asm src/boot/isr_stubs.asm
KERNEL_SRC := src/kernel/main.c src/kernel/idt.c src/kernel/pic.c
LINKER := src/kernel/linker.ld

# Expected output objs
BOOT_OBJS	:= $(BUILD)/boot.o $(BUILD)/isr_stubs.o
KERNEL_OBJS	:= $(BUILD)/main.o $(BUILD)/idt.o $(BUILD)/pic.o
OBJS        := $(BOOT_OBJS) $(KERNEL_OBJS)

# Compiler/Linker flags
CFLAGS  := -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m32 -O2 \
			-Wall -Wextra -Wno-pointer-to-int-cast -Wno-unused-parameter

LDFLAGS := -nostdlib -z max-page-size=0x1000 -T $(LINKER)

# -----------------------------------------------------------------------------
# Building targets/rules
# -----------------------------------------------------------------------------

all: $(KERNEL)

$(BUILD):
	@mkdir -pv $(BUILD)

# Assemble NASM sources
$(BUILD)/%.o: src/boot/%.asm | $(BUILD)
	$(AS) -f elf32 $< -o $@

# Compile C sources
$(BUILD)/%.o: src/kernel/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel ELF
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# -----------------------------------------------------------------------------
# Create the ISO image
# -----------------------------------------------------------------------------

iso: $(KERNEL)
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	cp ./grub.cfg $(BUILD)/iso/boot/grub/
	grub-mkrescue -o $(ISO) $(BUILD)/iso

# -----------------------------------------------------------------------------
# Run ISO inside QEMU
# -----------------------------------------------------------------------------
run: iso
	qemu-system-i386 -cdrom $(ISO) -serial stdio

debug: iso
	qemu-system-i386 -cdrom $(ISO) -s -S -serial stdio -display default

# -----------------------------------------------------------------------------
# Utilities
# -----------------------------------------------------------------------------
clean:
	@rm -rf $(BUILD)
	@rm -rf iso

rebuild: clean all

.PHONY: all clean rebuild iso run