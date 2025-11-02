# HorizonOS

[![GitHub top language](https://img.shields.io/github/languages/top/horizonos-project/horizon?logo=c&label=)](https://github.com/horizonos-project/horizon/blob/master/src/kernel/Makefile)
[![GitHub license](https://img.shields.io/github/license/horizonos-project/horizon)](https://github.com/horizonos-project/horizon/blob/master/LICENSE)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/horizonos-project/horizon)](https://github.com/horizonos-project/horizon/commits)

Horizon OS (sometimes 'HorizonOS' or 'Horizon') is a 32-bit x86 Operating System. Horizon is written mostly in C and Intel Assembly. This exists as a side project, something to work on semi-frequently to see if something interesting can come from it.

The plan for this system in the future is to be POSIX compliant.

## Current Features

Currently, Horizon has the following features;
 - Booting into 32-bit Protected Mode
 - Setting up a basic IDT and PIC
 - Correctly sets up a Stack and hands off to C

And that... is it. It boots, and that's about it for the time being. Over the course of the months of November and December, there are plans to have a semi-stable userland by the beginning of 2026.

## Project Status

Horizon is in **very early kernel development**.  
Core goals for the next releases include:

- Interrupt handling (keyboard + timer)
- Memory manager groundwork
- Minimal userspace shell
- Potential HorizonOS-specific compiler toolchain

## How to Compile

> [!WARNING]
> This project will **NOT** compile on Windows or MSYS environments. Please use WSL or Linux as a host.

Before you build this project, ensure you have the following tools:

| Tool | Version | Notes |
| ---- | ------- | ----- |
| Clang | $\geq$ 18.1.3  | Compiler for C/C++         |
| NASM  | $\geq$ 2.16.01 | Intel-compatible assembler |
| make  | $\geq$ 4.3     | Build system               |

Horizon is built with `make` since it's a universal build system.
 - To build the plain *ELF* file, run `make`
 - To build the ISO, run `make iso`
 - To run/rebuild an ISO and put it into qemu, run `make run`
 - To debug the kernel, run `make debug` along with `gdb`
    * *Note: QEMU exposes debugging on port 1234*

<!-- Other tests and systems will be added with time -->
