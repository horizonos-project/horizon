# Horizon Makefile
# (c) 2025 - HorizonOS Project
#
# https://open.spotify.com/track/7ehXSFlAsbadnSM93Pj4jT?si=8063e959cb184181

GREEN    := \033[1;32m
BLUE     := \033[1;34m
YELLOW   := \033[1;33m
RED      := \033[1;31m
RESET    := \033[0m

VERSION     := 0.02.03
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
	AS 		:= nasm
	AR		:= i686-elf-ar
	OBJC 	:= i686-elf-objcopy
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
BOOT_SRC	:= $(SRC_D)/boot/boot.asm
LIBK_SRC	:= $(shell find $(SRC_D)/libk -type f -name '*.c' 2>/dev/null)
KERNEL_SRC	:= $(shell find $(SRC_D) -type f -name '*.c' ! -path "$(SRC_D)/libk/*")

BOOT_OBJS   := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(BOOT_SRC:.asm=.o))
KERNEL_OBJS := $(patsubst $(SRC_D)/%, $(BUILD)/%, $(KERNEL_SRC:.c=.o))
LIBK_OBJS	:= $(patsubst $(SRC_D)/%, $(BUILD)/%, $(LIBK_SRC:.c=.o))
LIBK_A		:= $(BUILD)/libk/libk.a
OBJS        := $(BOOT_OBJS) $(LIBK_OBJS) $(KERNEL_OBJS)

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------
CFLAGS  := -ffreestanding -fno-stack-protector -fno-pic -fno-pie -m32 -O2 \
			-fno-omit-frame-pointer -fno-builtin -Wall -Wextra -Wpedantic \
			-Wno-unused-function -Wno-unused-parameter -Wno-pointer-to-int-cast \
			-I$(SRC_D) -isystem $(shell $(CC) -print-file-name=include) \
			-nostdinc -mno-sse -mno-sse2 -mno-mmx -mno-3dnow \
			-Werror=implicit-function-declaration \
			-DHORIZON_VERSION=\"$(VERSION)\" \
			-DHORIZON_BUILD_DATE=\"$(BUILD_DATE)\"

LDFLAGS := -m elf_i386 -nostdlib -z max-page-size=0x1000 -T $(LINKER)

# -----------------------------------------------------------------------------
# Toolchain sanity check
# -----------------------------------------------------------------------------
check-tools:
	@command -v $(CC) >/dev/null 2>&1 || { \
		printf "$(RED)[ERR]$(RESET) Missing tool: $(CC)\n"; \
		printf "       Install it via 'yay -S i686-elf-gcc'\n"; exit 1; }
	@command -v $(LD) >/dev/null 2>&1 || { \
		printf "$(RED)[ERR]$(RESET) Missing tool: $(LD)\n"; \
		printf "       Install it via 'yay -S i686-elf-binutils'\n"; exit 1; }
	@command -v $(AS) >/dev/null 2>&1 || { \
		printf "$(RED)[ERR]$(RESET) Missing tool: $(AS)\n"; \
		printf "       Install it via 'yay -S nasm'\n"; exit 1; }
	@printf "$(GREEN)[OK!]$(RESET) Toolchain found and ready.\n"

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------
all: check-tools libk $(KERNEL)

$(BUILD):
	$(Q)mkdir -pv $(BUILD)

# Assemble boot sources
$(BUILD)/%.o: $(SRC_D)/%.asm
	$(Q)mkdir -p $(@D)
	@printf "$(BLUE)[ASM]$(RESET) %s\n" "$<"
	$(Q)$(AS) -f elf32 $< -o $@ 2> build/last_build.log || { \
		printf "$(RED)[ERR]$(RESET) %s failed:\n" "$<"; \
		cat build/last_build.log; \
		$(AS) -f elf32 $< -o $@; exit 1; }


# Compile C sources
$(BUILD)/%.o: $(SRC_D)/%.c
	$(Q)mkdir -p $(@D)
	@printf "$(YELLOW)[CC ]$(RESET) %s\n" "$<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@ 2> build/last_build.log || { \
		printf "$(RED)[ERR]$(RESET) %s failed:\n" "$<"; \
		cat build/last_build.log; \
		$(CC) $(CFLAGS) -c $< -o $@; exit 1; }

# Build libk (LibKernel) as a static archive
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

libk: $(LIBK_A)
	@printf "$(GREEN)[OK!]$(RESET) libk.a built successfully.\n"

# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------
raw: $(KERNEL)
	@printf "$(GREEN)[OK!]$(RESET) Kernel image built!\n"

iso: $(KERNEL)
	@mkdir -p $(BUILD)/iso/boot/grub
	@cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	@cp ./grub.cfg $(BUILD)/iso/boot/grub/
	@grub-mkrescue -o $(ISO) $(BUILD)/iso

run: check-tools raw
	@printf "$(BLUE)[RUN]$(RESET) Running kernel in qemu-system-i386\n"
	@qemu-system-i386 -kernel $(KERNEL) -serial stdio

debug: raw
	@printf "$(BLUE)[DBG]$(RESET) Debugging kernel in qemu-system-i386\n"
	@qemu-system-i386 -kernel $(KERNEL) -s -S -serial stdio -display default

clean:
	@rm -rf $(BUILD)
	@printf "$(RED)[CLEAN]$(RESET) Build artifacts removed\n"

rebuild: clean all

.PHONY: all clean rebuild iso run debug raw
