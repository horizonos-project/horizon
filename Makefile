# Horizon Makefile
# (c) 2025 - HorizonOS Project
#
# https://open.spotify.com/track/7ehXSFlAsbadnSM93Pj4jT?si=8063e959cb184181

GREEN    := \033[1;32m
BLUE     := \033[1;34m
YELLOW   := \033[1;33m
RED      := \033[1;31m
RESET    := \033[0m

VERSION     := 0.03.00
BUILD_DATE  := $(shell date +'%Y-%m-%d_%H:%M:%S')

# -----------------------------------------------------------------------------
# Optional verbosity toggle
# -----------------------------------------------------------------------------
VERBOSE ?= 0
ifeq ($(VERBOSE),1)
  Q :=
else
  Q := @
endif

# -----------------------------------------------------------------------------
# Tools (and OS detection)
# -----------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
	$(error Windows is not supported. Please use WSL or a Linux VM.)
else
	CC 		:= i686-elf-gcc
	LD 		:= i686-elf-ld
	AS 		:= i686-elf-as
	AR		:= i686-elf-ar
	OBJC 	:= i686-elf-objcopy
endif

ifeq ($(shell uname -s),Darwin)
	RESCUE = i686-elf-grub-mkrescue
else
	RESCUE = grub-mkrescue
endif

# -----------------------------------------------------------------------------
# Paths
# -----------------------------------------------------------------------------
SRC_D	:= src
BUILD 	:= build
ISO 	:= $(BUILD)/Horizon.iso
KERNEL 	:= $(BUILD)/kernel.elf
LINKER	:= $(SRC_D)/kernel/linker.ld


# -----------------------------------------------------------------------------
# Source discovery (which is super helpful)
# -----------------------------------------------------------------------------
BOOT_SRC	:= $(SRC_D)/boot/boot.S $(SRC_D)/kernel/isr_stubs.S $(SRC_D)/kernel/syscall/syscall_asm.S
BOOT_SRC	+= src/kernel/gdt_asm.S
LIBK_SRC	:= $(shell find $(SRC_D)/libk -type f -name '*.c' 2>/dev/null)
KERNEL_SRC 	:= $(shell find $(SRC_D) -type f -name '*.c' -not -path "$(SRC_D)/libk/*" 2>/dev/null)
INITRAMFS	:= build/initramfs.o

# The many iterations of this src only to learn that I needed to rename isr.asm 
# KERNEL_SRC 	:= $(shell find $(SRC_D)/kernel $(SRC_D)/mm $(SRC_D)/drivers -type f -name '*.c' 2>/dev/null)
# KERNEL_SRC 	:= $(shell find $(SRC_D)/kernel $(SRC_D)/mm -type f -name '*.c' 2>/dev/null)
# KERNEL_SRC 	:= $(shell find $(SRC_D) -type f -name '*.c' ! -path "$(SRC_D)/libk/*" 2>/dev/null)
# KERNEL_SRC	:= $(shell find $(SRC_D) -type f -name '*.c' ! -path "$(SRC_D)/libk/*")

BOOT_OBJS   := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(BOOT_SRC:.S=.o))
KERNEL_OBJS := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(KERNEL_SRC:.c=.o))
LIBK_OBJS	:= $(patsubst $(SRC_D)/%, $(BUILD)/%, $(LIBK_SRC:.c=.o))
LIBK_A		:= $(BUILD)/libk/libk.a
OBJS        := $(BOOT_OBJS) $(LIBK_OBJS) $(KERNEL_OBJS) $(INITRAMFS)

# -----------------------------------------------------------------------------
# Flags (optimized for pedantic compiling, legacy asm instructions and 32 bit)
# -----------------------------------------------------------------------------
CFLAGS  := -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m32 -O1 \
			-fno-omit-frame-pointer -fno-builtin -Wall -Wextra -Wpedantic \
			-Wno-unused-function -Wno-unused-parameter -Wno-pointer-to-int-cast \
			-I$(SRC_D) -isystem $(shell $(CC) -print-file-name=include) \
			-nostdinc -mno-sse -mno-sse2 -mno-mmx -mno-3dnow \
			-Werror=implicit-function-declaration -Werror=return-type \
			-Werror=incompatible-pointer-types -g -Wstrict-prototypes \
			-DHORIZON_VERSION=\"$(VERSION)\" \
			-DHORIZON_BUILD_DATE=\"$(BUILD_DATE)\"

LDFLAGS := -m elf_i386 -nostdlib -z max-page-size=0x1000 -T $(LINKER)

# -----------------------------------------------------------------------------
# Utility to see all the sources that Make picked up on
# -----------------------------------------------------------------------------
list-src:
	@printf "$(BLUE)--- Source Discovery ---$(RESET)\n"
	@printf "$(YELLOW)Boot ASM:$(RESET)\n"
	@printf "  %s\n" $(BOOT_SRC)
	@echo
	@printf "$(YELLOW)Kernel C:$(RESET)\n"
	@printf "  %s\n" $(KERNEL_SRC)
	@echo
	@printf "$(YELLOW)libk:$(RESET)\n"
	@printf "  %s\n" $(LIBK_SRC)
	@echo
	@printf "$(GREEN)--- Object targets ---$(RESET)\n"
	@printf "  %s\n" $(OBJS)

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------
all: libk $(KERNEL)

$(BUILD):
	$(Q)mkdir -pv $(BUILD)

# Assemble boot sources
$(BUILD)/%.o: $(SRC_D)/%.S
	$(Q)mkdir -p $(@D)
	@printf "$(BLUE)[ASM]$(RESET) %s\n" "$<"
	$(Q)$(AS) --32 $< -o $@ 2> build/last_build.log || { \
		printf "$(RED)[ERR]$(RESET) %s failed:\n" "$<"; \
		cat build/last_build.log; \
		exit 1; }


# Compile C sources
$(BUILD)/%.o: $(SRC_D)/%.c
	$(Q)mkdir -p $(@D)
	@printf "$(YELLOW)[CC ]$(RESET) %s\n" "$<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@ 2> build/last_build.log || { \
		printf "$(RED)[ERR]$(RESET) %s failed:\n" "$<"; \
		cat build/last_build.log; \
		$(CC) $(CFLAGS) -c $< -o $@; exit 1; }

# Build libk (LibKernel) as a static archive
# NOTE: libk is NOT a libc, it is the freestanding library that the kernel
# 		itself depends on, this is absolutely required for compilation
$(LIBK_A): $(LIBK_OBJS)
	$(Q)mkdir -p $(@D)
	@printf "$(BLUE)[AR ]$(RESET) %s\n" "$@"
	$(Q)$(AR) rcs $@ $^

# Link kernel ELF
$(KERNEL): $(OBJS)
	@printf "$(GREEN)[LD ]$(RESET) %s\n" "$@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ 2> build/last_build.log || { \
		printf "$(RED)[ERR]$(RESET) Linking failed:\n"; \
		cat build/last_build.log; \
		$(LD) $(LDFLAGS) -o $@ $^; exit 1; }
	@printf "$(GREEN)[OK!]$(RESET) Kernel linked -> $(KERNEL)\n"
	@rm -f *.tar

libk: $(LIBK_A)
	@printf "$(GREEN)[OK!]$(RESET) libk.a built successfully.\n"

$(INITRAMFS): initramfs.tar
	@printf "$(BLUE)[INITRAMFS]$(RESET) Creating initramfs object\n"
	$(Q)i686-elf-objcopy -I binary -O elf32-i386 -B i386 \
		--rename-section .data=.initramfs,alloc,load,readonly,data,contents \
		initramfs.tar $@

initramfs.tar: build_initramfs.sh
	@printf "$(BLUE)[TAR]$(RESET) Building initramfs\n"
	$(Q)./build_initramfs.sh

# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------
raw: $(KERNEL)
	@printf "$(GREEN)[OK!]$(RESET) Kernel image built!\n"

iso: $(KERNEL)
	@mkdir -p $(BUILD)/iso/boot/grub
	@cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	@cp ./grub.cfg $(BUILD)/iso/boot/grub/
	@$(RESCUE) -o $(ISO) $(BUILD)/iso

run: raw
	@clear
	@printf "$(BLUE)[RUN]$(RESET) Running kernel in qemu-system-i386...\n"
	@qemu-system-i386 -kernel $(KERNEL) -m 128M \
		-serial stdio -display default \
		-no-reboot -no-shutdown

run-iso: iso
	@clear
	@printf "$(BLUE)[RUN]$(RESET) Running ISO fin qemu-system-i386...\n"
	@qemu-system-i386 -cdrom $(ISO) -m 128M \
	-serial stdio -display default \
	-no-reboot -no-shutdown

debug: raw
	@printf "$(BLUE)[DBG]$(RESET) Debugging kernel in qemu-system-i386...\n"
	@qemu-system-i386 -kernel $(KERNEL) -s -S -m 128M \
		-serial stdio -display default \
		-no-reboot -no-shutdown

clean:
	@rm -rf ./initramfs.*
	@rm -rf $(BUILD)
	@rm -rf $(ISO)
	@rm -rf ./initramfs.*
	@printf "$(RED)[CLEAN]$(RESET) Build artifacts removed\n"

rebuild: clean all

.PHONY: all clean rebuild iso run debug raw
