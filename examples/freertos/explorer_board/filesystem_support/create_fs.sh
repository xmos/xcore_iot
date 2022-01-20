#!/bin/sh

XCORE_SDK_REPO_PATH=$(git rev-parse --show-toplevel)
WF200_FW=$XCORE_SDK_REPO_PATH/modules/rtos/drivers/wifi/sl_wf200/thirdparty/wfx-firmware/wfm_wf200_C0.sec

# Create directory for intended files and Copy renamed files into directory
tmp_dir=$(mktemp -d)
fat_mnt_dir=$tmp_dir
mkdir -p $fat_mnt_dir

mkdir $fat_mnt_dir/firmware
mkdir $fat_mnt_dir/crypto
mkdir $fat_mnt_dir/server
mkdir $fat_mnt_dir/wifi
cp echo_client_certs/server.pem $fat_mnt_dir/crypto/ca.pem
cp echo_client_certs/client.pem $fat_mnt_dir/crypto/cert.pem
cp echo_client_certs/client.key $fat_mnt_dir/crypto/key.pem
cp board_server_certs/server.pem $fat_mnt_dir/server/ca.pem
cp board_server_certs/server.key $fat_mnt_dir/server/key.pem
cp $WF200_FW $fat_mnt_dir/firmware/wf200.sec

if [ ! -f networks.dat ]; then
    ./wifi_profile.py
fi
cp networks.dat $fat_mnt_dir/wifi

# Run fatfs_mkimage.exe on the directory to create filesystem file
fatfs_mkimage --input=$tmp_dir --output=fat.fs