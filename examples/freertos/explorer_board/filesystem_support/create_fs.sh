# Create an empty 1 MiB file
dd if=/dev/zero of=fat.fs bs=1024 count=1024

if [ "$UNAME" == "Linux" ] ; then
    sudo umount -q fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil detach fat_mnt
fi

# Create an empty FAT filesystem in it
mkfs.vfat -v -F12 -s1 -S4096 -n xcore_filesystem fat.fs

mkdir -p fat_mnt

WF200_FW=$XMOS_AIOT_SDK_PATH/modules/lib_soc/src/peripherals/bsp/wf200_driver/thirdparty/wfx-firmware/wfm_wf200_C0.sec

if [ "$UNAME" == "Linux" ] ; then
    sudo mount -o loop fat.fs fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil attach -imagekey diskimage-class=CRawDiskImage -mountpoint fat_mnt fat.fs
fi

sudo mkdir fat_mnt/firmware
sudo mkdir fat_mnt/crypto
sudo mkdir fat_mnt/server
sudo mkdir fat_mnt/wifi
sudo cp echo_client_certs/server.pem fat_mnt/crypto/ca.pem
sudo cp echo_client_certs/client.pem fat_mnt/crypto/cert.pem
sudo cp echo_client_certs/client.key fat_mnt/crypto/key.pem
sudo cp board_server_certs/server.pem fat_mnt/server/ca.pem
sudo cp board_server_certs/server.key fat_mnt/server/key.pem
sudo cp $WF200_FW fat_mnt/firmware/wf200.sec

if [ ! -f networks.dat ]; then
    ./wifi_profile.py
fi
sudo cp networks.dat fat_mnt/wifi

if [ "$UNAME" == "Linux" ] ; then
    sudo umount fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil detach fat_mnt
fi

sudo rm -rf fat_mnt
