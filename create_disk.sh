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
