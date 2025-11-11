# HorizonOS

[![GitHub top language](https://img.shields.io/github/languages/top/horizonos-project/horizon?logo=c&label=)](https://github.com/horizonos-project/horizon/blob/main/Makefile)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/horizonos-project/horizon)](https://github.com/horizonos-project/horizon/commits)

Horizon OS (sometimes 'HorizonOS' or 'Horizon') is a 32-bit Operating System. Horizon is written mostly in C and Intel Assembly. This exists as a side project, something to work on semi-frequently to see if something interesting can come from it.

The plan for this system in the future is to be POSIX compliant.

## Current Features

Currently, Horizon has the following features;
 - Booting into 32-bit Protected Mode
 - Registering the EXT2 filesystem
 - Correctly sets up a Stack and hands off to C

And that... is it. It boots, and that's about it for the time being. Over the course of the months of November and December, there are plans to have a semi-stable userland by the beginning of 2026.

## Project Status

Horizon is in **very early kernel development**.  
Core goals for the next releases include:

- Interrupt handling (keyboard + timer)
- Memory manager groundwork
- Minimal userspace shell
- Potential HorizonOS-specific compiler toolchain

## Project Versioning

Horizon uses a modified style of [Semantic Versioning](https://semver.org) that follows the structure; `X.YY.ZZ-A-[BT]`

**X** is the release version tag, if this changes, then massive updates are happening across large amounts of the OS and programs are expected to break or lose functionality unless they conform to the new APIs.

**YY** is the feature version tag, when new features are added (or old features are retired/modified) this will be incremented. Most programs will be fine, but some could break.

**ZZ** is the patch version tag, when things get small patches, this gets incremented. This shouldn't break most programs, but it could in rare cases.

**-A** is the bugfix tag, if a release has bugs, they'll be pushed under this tag and should *fix* issues rather than cause them.

**\[BT\]** is build tag, this is used for non-release releases like Release Candidate (rc) or Beta (b), or some such similar tag.
