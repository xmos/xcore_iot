# Get unix name for determining OS
UNAME=$(uname)

# Create an empty 1 MiB file
dd if=/dev/zero of=fat.fs bs=1024 count=1024

if [ "$UNAME" == "Linux" ] ; then
    MKFS_VFAT_PATH=/sbin
    sudo umount -q fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    MKFS_VFAT_PATH=/usr/local/sbin
    hdiutil detach fat_mnt
fi

# Create an empty FAT filesystem in it
$MKFS_VFAT_PATH/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_fs fat.fs

mkdir -p fat_mnt

WF200_FW=$XCORE_SDK_PATH/modules/rtos/drivers/wifi/sl_wf200/thirdparty/wfx-firmware/wfm_wf200_C0.sec
# Mount the filesystem
if [ "$UNAME" == "Linux" ] ; then
    sudo mount -o loop fat.fs fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil attach -imagekey diskimage-class=CRawDiskImage -mountpoint fat_mnt fat.fs
fi

# Copy files into filesystem
sudo mkdir fat_mnt/firmware
sudo mkdir fat_mnt/wifi
sudo cp $WF200_FW fat_mnt/firmware/wf200.sec

if [ ! -f networks.dat ]; then
    ./wifi_profile.py
fi
sudo cp networks.dat fat_mnt/wifi

# Unmount the filesystem
if [ "$UNAME" == "Linux" ] ; then
    sudo umount fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil detach fat_mnt
fi

# Cleanup
sudo rm -rf fat_mnt
