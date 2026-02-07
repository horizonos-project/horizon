# HorizonOS Development Environment Setup

This document describes how to build and run HorizonOS in an emulator.
Running on real hardware is currently untested and **not recommended**.

> [!CAUTION]
> HorizonOS is an experimental operating system kernel.  
> It is **not** suitable for daily use.

---

## Requirements

You will need the following tools installed on your system:

| Tool | Purpose |
|------|--------|
| GNU Make | Build system |
| QEMU (i386) | Emulator |
| i686-elf cross toolchain | Compiler, assembler, linker |
| GRUB (`mkrescue`) | ISO creation |
| xorriso | ISO 9660 writer |

---

## Cross Toolchain (Required)

HorizonOS **requires** an `i686-elf` cross toolchain.  
The host system compiler **will not work**.

Required binaries:

- `i686-elf-gcc`
- `i686-elf-ld`
- `i686-elf-as`
- `i686-elf-ar`
- `i686-elf-objcopy`

---

### Installing the Toolchain

#### Arch Linux
```sh
yay -S i686-elf-binutils i686-elf-gcc
```

#### Other Linux Distributions

Most distributions do **not** ship `i686-elf-*` packages.

Build the cross compiler manually following:
 * [https://wiki.osdev.org/GCC_Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)

This is the recommended and most reliable approach.

#### macOS

Homebrew support for `i686-elf-gcc` is inconsistent and may be outdated.

You will likely need to:

* build the toolchain manually, **or**
* use a prebuilt tap if available

Additionally, macOS users **must** install Docker for ext2 tooling support:

```sh
brew install docker
```

---

## ext2 Disk Image Support

HorizonOS boots from an **ext2 disk image**.

* On Linux, native `e2fsprogs` tools are typically available
* On macOS, ext2 utilities are accessed via Docker

Disk image creation is handled automatically by:

```sh
make disk
```

---

## QEMU

HorizonOS runs under `qemu-system-i386`.

### Linux

```sh
sudo apt install qemu-system-x86
```

### macOS

```sh
brew install qemu
```

---

## Building HorizonOS

HorizonOS uses **GNU Make** as its build system.

> We will not be switching build systems unless absolutely necessary.
<!-- Hint: this will never happen. Linux has been doing this since '91. -->

### Common Make Targets

| Command         | Description                  |
| --------------- | ---------------------------- |
| `make`          | Build kernel and libk        |
| `make raw`      | Build raw kernel ELF         |
| `make iso`      | Build bootable ISO           |
| `make disk`     | Create ext2 disk image       |
| `make run`      | Run kernel directly in QEMU  |
| `make run-iso`  | Run ISO in QEMU              |
| `make debug`    | Run with GDB stub            |
| `make clean`    | Remove build artifacts       |
| `make list-src` | Show discovered source files |

---

## Platform Notes

### Windows

Native Windows builds are **not supported**.

Use one of the following instead:

* Windows Subsystem for Linux (WSL2)
* A Linux virtual machine

Attempting to build with MSVC or Windows toolchains is unsupported and untested. You do so at your own risk.

---

### ARM

ARM is **not supported** at this time.

The project is currently focused on stabilizing x86 (32-bit) support first.

---

## Current Limitations

* 32-bit x86 only
* Single userspace process
* No scheduler yet
* ext2 is read-only
* `/bin/hello` is loaded directly from disk

<!--
binutils:
../binutils-2.45/configure \
    --target="$TARGET" \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror \

gcc:
../gcc-15.2.0/configure \                                              
  --target="$TARGET" \
  --prefix="$PREFIX" \
  --disable-nls \
  --enable-languages=c,c++ \
  --without-headers \
  --disable-shared \
  --disable-threads \
  --disable-libssp \
  --disable-libmudflap \
  --disable-libgomp \
  --disable-libquadmath \
  --disable-libatomic \
  --disable-multilib

-->