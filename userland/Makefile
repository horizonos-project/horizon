# HorizonOS Userland tests
#
# This assembles local asm files into flat bins for testing
# These are not required for the kernel, libk, or the OS as
# a whole. These are just tests whilst we transition toward
# a functional userland (note drafted on 11/11/2025)

ASM      := nasm
BUILD    := build
SRC_DIR  := .
ASM_SRC  := $(wildcard $(SRC_DIR)/*.asm)
BINS     := $(patsubst $(SRC_DIR)/%.asm, $(BUILD)/%.bin, $(ASM_SRC))

ASMFLAGS := -f bin

GREEN := \033[1;32m
BLUE  := \033[1;34m
RED   := \033[1;31m
RESET := \033[0m

.PHONY: all clean rebuild list-src

all: $(BUILD) $(BINS)
	@printf "$(GREEN)[OK!]$(RESET) Built %d userland binaries.\n" $(words $(BINS))

$(BUILD):
	@mkdir -p $(BUILD)

# Assemble each .asm file into its own .bin
$(BUILD)/%.bin: $(SRC_DIR)/%.asm
	@printf "$(BLUE)[ASM]$(RESET) %s -> %s\n" "$<" "$@"
	@$(ASM) $(ASMFLAGS) $< -o $@ 2> $(BUILD)/last_build.log || { \
		printf "$(RED)[ERR]$(RESET) %s failed:\n" "$<"; \
		cat $(BUILD)/last_build.log; exit 1; }

clean:
	@rm -rf $(BUILD)
	@printf "$(RED)[CLEAN]$(RESET) Userland artifacts removed\n"

rebuild: clean all

# Just lists the found source files for inspection
list-src:
	@printf "$(BLUE)[SRC]$(RESET) Found ASM sources:\n"
	@for f in $(ASM_SRC); do echo "  $$f"; done