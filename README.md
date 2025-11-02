# HorizonOS

Horizon OS (sometimes 'HorizonOS' or 'Horizon') is a partially UNIX-compatible x86 Operating System. Horizon is written mostly in C and Intel Assembly. This exists as a side project, something to work on semi-frequently to see if something interesting can come from it.

## Current Features

Currently, Horizon has the following features;
 - Booting into 32-bit Protected Mode
 - Correctly sets up a Stack and hands off to C

And that... is it. It boots, and that's about it for the time being. Over the course of the months of November and December, there are plans to have a semi-stable userland by the beginning of 2026.

## How to Compile

Horizon is built using the classic `make` build system. If you want to build just the raw kernel ELF you can run `make elf`. If you're looking to build an ISO, run `make iso`.

<!-- Other tests and systems will be added with time -->
