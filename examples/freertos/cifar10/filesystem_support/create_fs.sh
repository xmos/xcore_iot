# Create an empty 1 MiB file
dd if=/dev/zero of=fat.fs bs=1024 count=1024

sudo umount -q fat_mnt

# Create an empty FAT filesystem in it
/sbin/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_filesystem fat.fs

mkdir -p fat_mnt

sudo mount -o loop fat.fs fat_mnt
sudo cp test_inputs/airplane.bin fat_mnt/airplane.bin
sudo cp test_inputs/bird.bin fat_mnt/bird.bin
sudo cp test_inputs/cat.bin fat_mnt/cat.bin
sudo cp test_inputs/deer.bin fat_mnt/deer.bin
sudo cp test_inputs/frog.bin fat_mnt/frog.bin
sudo cp test_inputs/horse.bin fat_mnt/horse.bin
sudo cp test_inputs/truck.bin fat_mnt/truck.bin

sudo umount fat_mnt

rmdir fat_mnt

