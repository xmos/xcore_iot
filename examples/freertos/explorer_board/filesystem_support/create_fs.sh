#!/bin/sh

XCORE_SDK_REPO_PATH=$(git rev-parse --show-toplevel)

# Create directory for intended files
tmp_dir=$(mktemp -d)
fat_mnt_dir=$tmp_dir
mkdir -p $fat_mnt_dir

# Copy files into filesystem directory
mkdir $fat_mnt_dir/fs
cp ./demo.txt $fat_mnt_dir/fs/demo.txt

# Run fatfs_mkimage on the directory to create filesystem file
fatfs_mkimage --input=$tmp_dir --output=fat.fs
