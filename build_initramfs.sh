#!/bin/bash

mkdir -pv initramfs/{bin,etc}

echo "Hello from initramfs!" > initramfs/hello.txt
echo "Welcome to HorizonOS" > initramfs/etc/motd

if [ -f userland/hello.asm ]; then
    nasm -f bin userland/hello.asm -o initramfs/bin/hello
fi

cd initramfs
tar -cf ../initramfs.tar *
cd ..

i686-elf-objcopy -I binary -O elf32-i386 -B i386 \
    --rename-section .data=.initramfs,alloc,load,readonly,data,contents \
    initramfs.tar initramfs.o

echo "initramfs.o created successfully!"