# HorizonOS

[![GitHub top language](https://img.shields.io/github/languages/top/horizonos-project/horizon?logo=c&label=)](https://github.com/horizonos-project/horizon/blob/main/Makefile)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/horizonos-project/horizon)](https://github.com/horizonos-project/horizon/commits)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d694b6fd6b7440e3b4cf494f65a6ab71)](https://app.codacy.com/gh/horizonos-project/horizon/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

Horizon OS (sometimes 'HorizonOS' or 'Horizon') is a 32-bit Operating System. Horizon is written mostly in C and AT&T Assembly. This exists as a side project, something to work on semi-frequently to see if something interesting can come from it.

The plan for this system in the future is to be POSIX compliant and have a clean codebase.

<!-- as of 11/20/2025: the Codacy grade we have is C. This should be B at the minimum. -->  

## Current Features

Currently, Horizon has the following features;
 - Booting into 32-bit Protected Mode
 - Decompressing the initramfs tar archive
 - Sets up required memory and registers for userland
 - Runs in a ring3 userland context

And that... is it. It boots, and that's about it for the time being. Over the course of the months of November and December, there are plans to have a semi-stable userland by the beginning of 2026.

## Project Status

Horizon is in **early kernel development**.  
Core goals for the next releases include:

- User input
- Minimal userspace shell
- Reading/Writing to a filesystem (finally getting out of initramfs)
- HorizonOS Toolchains

## How to Build Horizon?

Building Horizon is fairly straightforward once you have the required tools for the job. It's explained in greater detail in the provided [SETUP](./SETUP.md) document!

## How can I Contribute?

If you want to contribute, check out the [CONTRIBUTING](./.github/CONTRIBUTING.md) document!

## Project Versioning

Horizon uses a modified style of [Semantic Versioning](https://semver.org) that follows the structure; `X.YY.ZZ-A-[BT]`

**X** is the release version tag, if this changes, then massive updates are happening across large amounts of the OS and programs are expected to break or lose functionality unless they conform to the new APIs.

**YY** is the feature version tag, when new features are added (or old features are retired/modified) this will be incremented. Most programs will be fine, but some could break.

**ZZ** is the patch version tag, when things get small patches, this gets incremented. This shouldn't break most programs, but it could in rare cases.

**-A** is the bugfix tag, if a release has bugs, they'll be pushed under this tag and should *fix* issues rather than cause them.

**\[BT\]** is build tag, this is used for non-release releases like Release Candidate (rc) or Beta (b), or some such similar tag.
