# Create an empty 1 MiB file
dd if=/dev/zero of=fat.fs bs=1024 count=1024

sudo umount -q fat_mnt

# Create an empty FAT filesystem in it
/sbin/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_filesystem fat.fs

mkdir -p fat_mnt

WF200_FW=../../../lib_soc/src/peripherals/bsp/wf200_driver/thirdparty/wfx-firmware/wfm_wf200_C0.sec
sudo mount -o loop fat.fs fat_mnt
sudo mkdir fat_mnt/firmware
sudo mkdir fat_mnt/crypto
sudo mkdir fat_mnt/wifi
sudo cp aws/ca.pem fat_mnt/crypto/ca.pem
sudo cp aws/client.pem fat_mnt/crypto/cert.pem
sudo cp aws/client.key fat_mnt/crypto/key.pem
sudo cp $WF200_FW fat_mnt/firmware/wf200.sec

if [ ! -f networks.dat ]; then
    ./wifi_profile.py
fi
sudo cp networks.dat fat_mnt/wifi
sudo umount fat_mnt

rmdir fat_mnt

