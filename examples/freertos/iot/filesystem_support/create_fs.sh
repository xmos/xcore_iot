#!/bin/sh

XCORE_SDK_REPO_PATH=$(git rev-parse --show-toplevel)
WF200_FW=$XCORE_SDK_REPO_PATH/modules/rtos/drivers/wifi/sl_wf200/thirdparty/wfx-firmware/wfm_wf200_C0.sec

# Create directory for intended files 
tmp_dir=$(mktemp -d)
fat_mnt_dir=$tmp_dir
mkdir -p $fat_mnt_dir

# Copy files into filesystem directory
mkdir $fat_mnt_dir/firmware
mkdir $fat_mnt_dir/crypto
mkdir $fat_mnt_dir/wifi
cp aws/ca.pem $fat_mnt_dir/crypto/ca.pem
cp aws/client.pem $fat_mnt_dir/crypto/cert.pem
cp aws/client.key $fat_mnt_dir/crypto/key.pem
cp $WF200_FW $fat_mnt_dir/firmware/wf200.sec

if [ ! -f networks.dat ]; then
    ./wifi_profile.py
fi
cp networks.dat $fat_mnt_dir/wifi

# Run fatfs_mkimage on the directory to create filesystem file
fatfs_mkimage --input=$tmp_dir --output=fat.fs