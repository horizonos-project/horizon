# Horizon Makefile
# (c) 2025- HorizonOS Project
#
# Building Horizon requires the following tools;
#
# clang         --> 18.1.3
# make          --> 4.3
# nasm          --> 2.16.01
# ld.lld        --> 18.1.3
# grub-mkrescue --> 2.12

# Tools
CC 		:= clang
LD 		:= ld.lld
AS 		:= nasm
OBJC 	:= llvm-objcopy

# Paths
SRC_D	:= src
BUILD 	:= build
ISO 	:= $(BUILD)/Horizon.iso
KERNEL 	:= $(BUILD)/kernel.elf
LINKER	:= $(SRC_D)/kernel/linker.ld

# -----------------------------------------------------------------------------
# Source discovering, saves us manually speccing files!
# -----------------------------------------------------------------------------

BOOT_SRC := $(SRC_D)/boot/boot.asm $(SRC_D)/boot/isr_stubs.asm
KERNEL_SRC := $(shell find $(SRC_D)/kernel -type f -name '*.c')

# November 2, 2025
# BOOT_SRC 	:= src/boot/boot.asm src/boot/isr_stubs.asm
# KERNEL_SRC 	:= src/kernel/main.c src/kernel/idt.c src/kernel/pic.c
# LINKER 		:= src/kernel/linker.ld

# Expected output objs
BOOT_OBJS  := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(BOOT_SRC:.asm=.o))
KERNEL_OBJS:= $(patsubst $(SRC_D)/%, $(BUILD)/%, $(KERNEL_SRC:.c=.o))
OBJS       := $(BOOT_OBJS) $(KERNEL_OBJS)

# -----------------------------------------------------------------------------
# Compiler/Linker flags
# -----------------------------------------------------------------------------

CFLAGS  := -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m32 -O2 \
			-Wall -Wextra -Wno-pointer-to-int-cast -Wno-unused-parameter \
			-Wno-static-in-inline

LDFLAGS := -nostdlib -z max-page-size=0x1000 -T $(LINKER)

# -----------------------------------------------------------------------------
# Building rules
# -----------------------------------------------------------------------------
all: $(KERNEL)

$(BUILD):
	@mkdir -pv $(BUILD)

# Assemble boot sources
$(BUILD)/%.o: $(SRC_D)/%.asm
	@mkdir -p $(@D)
	$(AS) -f elf32 $< -o $@

# Compile all C sources (recursive)
$(BUILD)/%.o: $(SRC_D)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Link final kernel ELF
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

rebuild: clean all

.PHONY: all clean rebuild iso run