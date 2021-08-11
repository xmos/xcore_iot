#!/bin/sh

# Get unix name for determining OS
UNAME=$(uname)

# Create an empty 1 MiB file
dd if=/dev/zero of=fat.fs bs=1024 count=1024

if [ "$UNAME" = "Linux" ] ; then
    MKFS_VFAT_PATH=/sbin
    sudo umount -q fat_mnt
elif [ "$UNAME" = "Darwin" ] ; then
    MKFS_VFAT_PATH=/usr/local/sbin
    hdiutil detach fat_mnt
fi

# Create an empty FAT filesystem in it
$MKFS_VFAT_PATH/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_fs fat.fs

mkdir -p fat_mnt

# Mount the filesystem
if [ "$UNAME" = "Linux" ] ; then
    sudo mount -o loop fat.fs fat_mnt
elif [ "$UNAME" = "Darwin" ] ; then
    hdiutil attach -imagekey diskimage-class=CRawDiskImage -mountpoint fat_mnt fat.fs
fi

# Copy files into filesystem
sudo cp test_inputs/airplane.bin fat_mnt/airplane.bin
sudo cp test_inputs/bird.bin fat_mnt/bird.bin
sudo cp test_inputs/cat.bin fat_mnt/cat.bin
sudo cp test_inputs/deer.bin fat_mnt/deer.bin
sudo cp test_inputs/frog.bin fat_mnt/frog.bin
sudo cp test_inputs/horse.bin fat_mnt/horse.bin
sudo cp test_inputs/truck.bin fat_mnt/truck.bin

# Unmount the filesystem
if [ "$UNAME" = "Linux" ] ; then
    sudo umount fat_mnt
elif [ "$UNAME" = "Darwin" ] ; then
    hdiutil detach fat_mnt
fi

# Cleanup
sudo rm -rf fat_mnt
