#!/bin/bash
# create_disk.sh - Create an ext2 disk image for HorizonOS testing
# Only works on Linux and other unix systems

set -e  # Exit on error

DISK_IMAGE="disk.img"
DISK_SIZE_MB=64
MOUNT_POINT="mnt"

echo "[disk] Creating ${DISK_SIZE_MB}MB disk image..."

# Create empty disk image
dd if=/dev/zero of=${DISK_IMAGE} bs=1M count=${DISK_SIZE_MB} 2>/dev/null

echo "[disk] Creating partition table..."

# Create MBR partition table with a single ext2 partition
# Using sfdisk for scriptable partitioning
cat > partition.txt << EOF
label: dos
type=83, bootable
EOF

sfdisk ${DISK_IMAGE} < partition.txt 2>/dev/null
rm partition.txt

echo "[disk] Setting up loop device..."

# Set up loop device for the disk image
LOOP_DEVICE=$(sudo losetup --find --show --partscan ${DISK_IMAGE})

echo "[disk] Loop device: ${LOOP_DEVICE}"

# Give the kernel a moment to create partition devices
sleep 1

# The first partition will be ${LOOP_DEVICE}p1
PARTITION="${LOOP_DEVICE}p1"

echo "[disk] Creating ext2 filesystem on ${PARTITION}..."

# Create ext2 filesystem on the partition
sudo mkfs.ext2 -F ${PARTITION}

echo "[disk] Mounting filesystem..."

# Create mount point and mount
mkdir -p ${MOUNT_POINT}
sudo mount ${PARTITION} ${MOUNT_POINT}

echo "[disk] Populating filesystem..."

# Create some test files and directories
sudo mkdir -p ${MOUNT_POINT}/bin
sudo mkdir -p ${MOUNT_POINT}/etc
sudo mkdir -p ${MOUNT_POINT}/home

# Copy test files (if they exist)
if [ -f "hello" ]; then
    sudo cp hello ${MOUNT_POINT}/bin/
fi

# Create a test file
echo "Hello from ext2!" | sudo tee ${MOUNT_POINT}/test.txt > /dev/null

# Create a MOTD
echo "HorizonOS ext2 filesystem - Now with real disk I/O!" | \
    sudo tee ${MOUNT_POINT}/etc/motd > /dev/null

echo "[disk] Filesystem contents:"
sudo ls -lR ${MOUNT_POINT}

echo "[disk] Unmounting..."

# Unmount
sudo umount ${MOUNT_POINT}

# Detach loop device
sudo losetup -d ${LOOP_DEVICE}

# Cleanup
rmdir ${MOUNT_POINT}

echo "[disk] ================================"
echo "[disk] Disk image created: ${DISK_IMAGE}"
echo "[disk] Size: ${DISK_SIZE_MB} MB"
echo "[disk] Format: MBR with ext2 partition"
echo "[disk] ================================"
echo ""
echo "To use with QEMU:"
echo "  qemu-system-i386 -kernel kernel.elf -hda ${DISK_IMAGE}"
echo ""
echo "To inspect MBR:"
echo "  xxd -l 512 ${DISK_IMAGE} | head -20"
echo ""
echo "To mount manually:"
echo "  sudo losetup -fP ${DISK_IMAGE}"
echo "  sudo mount /dev/loopXp1 mnt/"