#!/bin/env bash

set -e

docker run --rm \
  -v "$PWD/build:/work" \
  horizon-fs \
  bash -c '
    set -e

    DISK=/work/disk.img

    echo "[disk] Creating disk image"
    truncate -s 64M "$DISK"

    echo "[disk] Formatting ext2"
    mkfs.ext2 -F -I 128 -N 4096 "$DISK"

    echo "[disk] Creating directories"
    debugfs -w "$DISK" <<EOF
mkdir /bin
mkdir /etc
EOF

    echo "[disk] Writing files"
    debugfs -w "$DISK" <<EOF
write /work/hello /bin/hello
write /work/motd /etc/motd
write /work/test.txt /test.txt
EOF

    echo "[disk] Done"
'

docker run --rm \
  -v "$PWD:/repo:ro" \
  -v "$PWD/build:/work" \
  horizon-fs \
  bash -lc '
    set -euo pipefail
    test -f /repo/initramfs/bin/hello

    debugfs -w -R "write /repo/initramfs/bin/hello /bin/hello" /work/disk.img
    debugfs -w -R "write /repo/initramfs/etc/motd /etc/motd" /work/disk.img || true
    debugfs -w -R "write /repo/initramfs/hello.txt /hello.txt" /work/disk.img || true

    debugfs -R "ls -l /bin" /work/disk.img
  '

