# Horizon Makefile
# (c) 2025- HorizonOS Project
#

GREEN    := \033[1;32m
BLUE     := \033[1;34m
YELLOW   := \033[1;33m
RED      := \033[1;31m
RESET    := \033[0m

# -----------------------------------------------------------------------------
# Tools (And also OS detection)
# NOTES:
#	- MacOS users should use Homebrew for compilers/tools
#	- Windows is explicitly not supported
#	- Linux has most of this preinstalled (before cross tools)
# -----------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
	@printf "Windows is not capable of compiling this project.\n"
	@printf "Please use WSL or a Linux virual machine.\n"
	@printf "WSL Help: https://learn.microsoft.com/en-us/windows/wsl/\n"
	@false
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Darwin)
		# Darwin == MacOS (w/ homebrew installed)
		# brew install i686-elf-gcc !!
		CC 		:= i686-elf-gcc
		LD 		:= i686-elf-ld
		AS 		:= nasm
		OBJC 	:= i686-elf-objcopy
	else
		# Linux or other *nix like systems
		CC 		:= gcc
		LD 		:= ld
		AS 		:= nasm
		OBJC 	:= objcopy
	endif
endif

# Paths
SRC_D	:= src
BUILD 	:= build
ISO 	:= $(BUILD)/Horizon.iso
KERNEL 	:= $(BUILD)/kernel.elf
LINKER	:= $(SRC_D)/kernel/linker.ld

# -----------------------------------------------------------------------------
# Source discovering, saves us manually speccing files!
# -----------------------------------------------------------------------------

BOOT_SRC	:= $(SRC_D)/boot/boot.asm
LIBK_SRC	:= $(shell find $(SRC_D)/libk -type f -name '*.c' 2>/dev/null)
KERNEL_SRC	:= $(shell find $(SRC_D) -type f -name '*.c' ! -path "$(SRC_D)/libk/*")

# Expected output objs
BOOT_OBJS   := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(BOOT_SRC:.asm=.o))
KERNEL_OBJS := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(KERNEL_SRC:.c=.o))
LIBK_OBJS	:= $(patsubst $(SRC_D)/%, $(BUILD)/%, $(LIBK_SRC:.c=.o))
OBJS        := $(BOOT_OBJS) $(LIBK_OBJS) $(KERNEL_OBJS)

# -----------------------------------------------------------------------------
# Compiler/Linker flags
# -----------------------------------------------------------------------------

CFLAGS  := -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m32 -O2 \
			-fno-omit-frame-pointer -Wall -Wextra -Wno-pointer-to-int-cast \
			-Wno-unused-parameter -Wno-unused-function \
			-I$(SRC_D)

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
	@printf "$(BLUE)[ASM]$(RESET) %s\n" "$<"
	$(AS) -f elf32 $< -o $@

# Compile all C sources (recursive)
$(BUILD)/%.o: $(SRC_D)/%.c
	@mkdir -p $(@D)
	@printf "$(YELLOW)[CC ]$(RESET) %s\n" "$<"
	$(CC) $(CFLAGS) -c $< -o $@

# Link final kernel ELF
$(KERNEL): $(OBJS)
	@printf "$(GREEN)[LD ]$(RESET) %s\n" "$@"
	$(LD) $(LDFLAGS) -o $@ $^
	@printf "$(GREEN)[OK!]$(RESET) Kernel linked -> $(KERNEL)\n"

# -----------------------------------------------------------------------------
# Build only the raw kernel ELF
# -----------------------------------------------------------------------------
raw: $(KERNEL)
	@printf "$(GREEN)[OK!]$(RESET) Kernel image built!\n"
	@true

# -----------------------------------------------------------------------------
# Create the ISO image
# -----------------------------------------------------------------------------
iso: $(KERNEL)
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	cp ./grub.cfg $(BUILD)/iso/boot/grub/
	grub-mkrescue -o $(ISO) $(BUILD)/iso

# -----------------------------------------------------------------------------
# Run ISO inside QEMU (assuming kernel is built)
# -----------------------------------------------------------------------------
run: raw
	@printf "$(BLUE)[RUN]$(RESET) Running kernel in qemu-system-i386"
	qemu-system-i386 -kernel $(KERNEL) -serial stdio

debug: raw
	@printf "$(BLUE)[DBG]$(RESET) Debugging kernel in qemu-system-i386"
	qemu-system-i386 -kernel $(KERNEL) -s -S -serial stdio -display default

# -----------------------------------------------------------------------------
# Utilities
# -----------------------------------------------------------------------------
clean:
	@rm -rf $(BUILD)

rebuild: clean all

.PHONY: all clean rebuild iso run debug raw