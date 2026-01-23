#!/usr/bin/env bash
set -euo pipefail

# Paths
ROOT_DIR="init"
OUT_DIR="build"
IMG="${OUT_DIR}/disk.img"

IMG_SIZE="64M"

# --- Populate init/ tree ----------------------------------------------------
mkdir -pv "${ROOT_DIR}"/{sbin,bin,etc}

printf "Message of the Day: This commit is probably broken!.\n" > "${ROOT_DIR}/etc/motd"
printf "Welcome to HorizonOS!\n" > "${ROOT_DIR}/etc/welcome"

# Build init into the tree (note: output should be init/sbin/init)
rm -f "${ROOT_DIR}/sbin/init"
i686-elf-gcc \
  -nostdinc -nostdlib -ffreestanding \
  -m32 -Ttext=0x00400000 -o "${ROOT_DIR}/sbin/init" \
  init/init.c

# --- Make ext2 image as a raw "whole disk" ---------------------------------
mkdir -p "${OUT_DIR}"
rm -f "${IMG}"
truncate -s "${IMG_SIZE}" "${IMG}"

# This copies the directory tree directly into the new filesystem.
mkfs.ext2 -F -I 128 -N 4096 -d "${ROOT_DIR}" "${IMG}"

echo "[ok] Wrote ext2 rootfs to ${IMG} (size ${IMG_SIZE})"
