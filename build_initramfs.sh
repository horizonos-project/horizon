#!/bin/bash

rm -rf initramfs
mkdir -pv initramfs/{bin,etc}

echo "Hello from initramfs!" > initramfs/hello.txt
echo "Welcome to HorizonOS, from initramfs?" > initramfs/etc/motd

if [ -f userland/hello.c ]; then
    i686-elf-gcc -nostdlib -nostdinc -ffreestanding -m32 \
        -Ttext=0x00400000 \
        -o hello \
        userland/hello.c

    if [ -f hello ]; then
        echo "Copying hello to initramfs/bin/"
        cp hello initramfs/bin/hello
    else
        echo "ERROR: hello binary not created!"
        exit 1
    fi
fi

cd initramfs
tar --format=ustar --exclude='._*' --exclude='.DS_Store' -cf ../initramfs.tar *
cd ..

i686-elf-objcopy -I binary -O elf32-i386 -B i386 \
    --rename-section .data=.initramfs,alloc,load,readonly,data,contents \
    initramfs.tar initramfs.o

echo "initramfs.o created successfully!"