# Create an empty 1 MiB file
dd if=/dev/zero of=fat.fs bs=1024 count=1024

# Create an empty FAT filesystem in it
/sbin/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_filesystem fat.fs

mkdir -p fat_mnt

sudo mount -o loop fat.fs fat_mnt
sudo mkdir fat_mnt/firmware
sudo cp wfm_wf200_C0.sec fat_mnt/firmware/wf200.sec
sudo sh -c 'echo "Hello, World!" > fat_mnt/hello.txt'
sudo umount fat_mnt

rmdir fat_mnt

