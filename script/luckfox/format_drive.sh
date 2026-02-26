#!/bin/bash

# Script version using fdisk

set -e

# Check root privileges
if [[ $EUID -ne 0 ]]; then
   echo "Error: This script must be run as root"
   exit 1
fi

DISK="/dev/mmcblk1"  # Modify to your disk

# Check if /mnt/sdcard is mounted and unmount it
if mountpoint -q /mnt/sdcard; then
    echo "/mnt/sdcard is mounted, unmounting..."
    umount /mnt/sdcard
    echo "Unmounted successfully."
fi

# Check if the disk is mounted elsewhere and unmount it
if mount | grep -q "${DISK}p1"; then
    echo "Partition ${DISK}p1 is mounted, unmounting..."
    umount ${DISK}p1
    echo "Unmounted successfully."
fi

# Use fdisk non-interactive operation
echo "Deleting and recreating partition..."
fdisk $DISK << EOF
o
n
p
1



w
EOF

# Wait for the system to recognize the new partition
sleep 2

# Format with exFAT by default
echo "Formatting with exFAT filesystem..."
mkfs.exfat ${DISK}p1

echo "Format complete!"

# Display partition information
echo "Current partition information:"
fdisk -l $DISK

echo "Complete!"