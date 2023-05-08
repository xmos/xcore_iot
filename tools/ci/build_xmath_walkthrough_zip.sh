#!/bin/bash
set -e
set -x

# get the repo and dependencies
west init -m https://github.com/xmos/xmath_walkthrough/
west update

# make zip archive
zip -r --symlinks xmath_walkthrough.zip xmath_walkthrough

# cleanup
rm -rf xmath_walkthrough
rm -rf .west
