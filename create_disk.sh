#!/bin/bash
set -e

DISK_IMAGE="disk.img"
DISK_SIZE_MB=64
MOUNT_DIR="/mnt/horizon"

# NOTE: This will probably be removed once persistant storage exists.
#       (once we can write to the ext2 filesystem)
if [ ! -f "$DISK_IMAGE" ]; then
    truncate -s ${DISK_SIZE_MB}M "$DISK_IMAGE"
    mkfs.ext2 -F -I 128 -N 4096 "$DISK_IMAGE"

    sudo mkdir -pv "$MOUNT_DIR"
    sudo mount -o loop "$DISK_IMAGE" "$MOUNT_DIR"
    sudo chown -R $USER:$USER /mnt/horizon

    mkdir -p "$MOUNT_DIR/bin" "$MOUNT_DIR/etc"
    echo "Welcome to Horizon!" > "$MOUNT_DIR/etc/motd"
    cp hello "$MOUNT_DIR/bin/"
    echo "This is a text file, woah!" > "$MOUNT_DIR/test.txt"

    sync
    sudo umount "$MOUNT_DIR"

    cp "$DISK_IMAGE" build/
else
    cp "$DISK_IMAGE" build/
    echo "Copied existing image to build directory."
fi