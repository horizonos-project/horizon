# HorizonOS

[![GitHub top language](https://img.shields.io/github/languages/top/horizonos-project/horizon?logo=c&label=)](https://github.com/horizonos-project/horizon/blob/main/Makefile)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/horizonos-project/horizon)](https://github.com/horizonos-project/horizon/commits)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/horizonos-project/horizon)


Horizon OS (sometimes 'HorizonOS' or 'Horizon') is a 32-bit Operating System. Horizon is almost entirely written in C with some AT&T Assembly as needed. This exists as a side project, something to work on semi-frequently to see if something interesting can come from it. My hopes are high that this will grow over time!

The original plan for this OS project was to be **very** in-line with POSIX (IEEE 1003.1-2024). Accomplishing that is incredibly challenging for a project of this small scope, so the goal has been shifted back from "POSIX-Certified" (Meaning that the operating system has been certified to conform to one or more of the various POSIX standards.) to "Mostly POSIX Compliant", much like Linux, alongside a suite of other operating systems.

> [!CAUTION]
> Horizon is not intended to replace any existing operating system, nor is it suitable for production use.

## Current Features

Currently, Horizon has the following features;
 - Booting into 32-bit Protected Mode
 - Detects and mounts a read-only ext2 filesystem from an ATA disk
 - Sets up required memory and registers for userland
 - Runs **user** programs in a ring3 **(userspace)** context

And that... is it. It boots, it runs a single binary and gracefully stops.

## Project Status

Horizon is in **early kernel development**.  
Core goals for the next releases include:

- User input
- Minimal userspace shell
- Writing to ext2
- HorizonOS Toolchains
   * Such toolchains include a compiler and libc to bootstrap future apps

## How to Build Horizon?

Building Horizon is fairly straightforward once you have the required tools for the job. It's explained in greater detail in the provided [SETUP](./SETUP.md) document!

## How can I Contribute?

If you want to contribute, check out the [CONTRIBUTING](./.github/CONTRIBUTING.md) document! It has all the info you need to get started with developing HorizonOS and its ecosystem!

## Project Versioning

Horizon uses a modified style of [Semantic Versioning](https://semver.org) that follows the structure; `X.YY.ZZ-A`

**X** is the release version tag, if this changes, then massive updates are happening across large amounts of the OS and programs are expected to break or lose functionality unless they conform to the new APIs.

**YY** is the minor release tag, when enough new features are added (or old features are retired/modified) this will be incremented. Most programs will be fine, but some could break. From the deprecation/removal process.

**ZZ** is the patch version tag, when things get small patches, or functions get refactored and lots of small changes happen at once; this gets incremented. This shouldn't break most programs, but it could in rare cases. Though mostly at the kernel level and not at the userland level.

> [!NOTE]
> **-A** is the bugfix tag, if a release has bugs, they'll be pushed under this tag and should *fix* issues rather than cause them. This will be used starting at the first major release of Horizon. 1.00.00.
