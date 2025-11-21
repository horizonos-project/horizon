# Horizon Live Environment Setup Guide

To set up HorizonOS for use in an emulator such as qemu (recommended) or on actual hardware (untested) via building from the source, you will need the following tools to make this possible.

> [!CAUTION]
> This project is not in a state that one should use it as a daily-use operating system. Please do not use this for daily tasks!

| Tool | Version | Notes |
| ---- | ------- | ----- |
| GCC     | $\geq$ 11.2.0   | Compiler and Assembler     |
| make    | $\geq$ 4.3      | Build system               |
| grub    | latest          | Required for mkrescue      |
| xorriso | latest          | ISO 9660 writer            |

---

For GCC, it's recommended to use an `i686` or `i386` ELF compiler, these can usually be installed by a package manager;

> **Arch Linux:** `sudo yay i686-elf-binutils i686-elf-gcc`

> **Debian:** `sudo apt-get install i686-elf-binutils i686-elf-gcc -y`

> **MacOS:** `brew install i686-elf-gcc`

Compilation and assembly are unified under `i686-elf-*` to minimize dependencies and ensure complete compatibility. This will remain the case in the future once Horizon is self-hosted or building under a different compiler.

Make comes preinstalled across Linux and MacOS.

### How about Windows?

Unless you use Windows Subsystem for Linux, this project is not compilable in any Windows x64 or x86 environments. Windows is not built for projects of this nature. Using either a Linux subsystem or a virtual machine is preferred.

### How about ARM?

ARM is not supported at this time, we can barely handle x86, arm64 might blow up the entire project. It may be considered in the far future once this project has more contributors and has more hardware that it can be tested on.

# Building the Project

This project uses Make, since it's insanely straightforward and borderline universal. We will not be switching to another buildsystem unless it becomes absolutely needed.

<!-- Hint, it won't. Linux has been doing this since '91 -->

| Build cmd | What it does |
| --- | --- |
| `make raw` | Builds the raw kernel ELF without packing into an ISO. |
| `make iso` | Builds the kernel and packages it into an ISO alongside some GRUB data. |
| `make run` | Runs the kernel inside `qemu-system-i386` (builds the kernel if not found) |
| `make run-iso` | Runs the ISO in `qemu-system-i386` (will build the ISO if missing) |
| `make debug` | Runs the kernel in the qemu system but in debug mode. |
| `make rebuild` | Cleans the build directory and rebuilds the kernel ELF. |
| `make list-src` | Lists all the source files and the targets in the Makefile. |
| `make clean` | Cleans out the build directory. |
