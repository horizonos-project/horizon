# Horizon Live Environment Setup Guide

To set up HorizonOS for use in an emulator such as qemu (recommended) or on actual hardware (untested) via building from the source, you will need the following tools to make this possible.

| Tool | Version | Notes |
| ---- | ------- | ----- |
| GCC     | $\geq$ 15.2.0   | Compiler for C             |
| NASM    | $\geq$ 2.16.01  | Intel-compatible assembler |
| make    | $\geq$ 4.3      | Build system               |
| grub    | latest          | Required for mkrescue      |
| xorriso | latest          | ISO 9660 writer            |

---

For GCC, it's recommended to use an `i686` or `i386` ELF compiler, these can usually be installed by a package manager;

> **Arch Linux:** `sudo pacman -S i686-elf-gcc`

> **Debian:** `sudo apt-get install i686-elf-gcc -y`

> **MacOS:** `brew install i686-elf-gcc`

NASM was chosen since it's a portable x86 assembler that can build flat ELF32 assembly using the preferred syntax of this project, Intel Syntax. AT&T Syntax ASM is generally discouraged in this project.

Make comes preinstalled across Linux and MacOS. 

### How about Windows?

Unless you use Windows Subsystem for Linux, this project is not compilable in any Windows x64 or x86 environments.

### How about ARM?

ARM is not supported at this time, we can barely handle x86, arm64 might blow up the entire project.

# Building the Project

| Build cmd | What it does |
| --- | --- |
| `make raw` | Builds the raw kernel ELF without packing into an ISO. |
| `make iso` | Builds the kernel and packages it into an ISO alongside some GRUB data. |
| `make run` | Runs the kernel inside `qemu-system-i386` (builds the kernel if not found) |
| `make debug` | Runs the kernel in the qemu system but in debug mode. |
| `make rebuild` | Cleans the build directory and rebuilds the kernel ELF. |
| `make clean` | Cleans out the build directory. |