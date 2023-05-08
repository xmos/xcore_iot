#!/bin/bash
set -e
set -x

#docker run --rm -u $(id -u):$(id -g) -w /xmath_walkthrough -v ${{github.workspace}}:/xmath_walkthrough ${XCORE_BUILDER} bash -l tools/ci/build_xmath_walkthrough_zip.sh

#docker run --rm -u $(id -u):$(id -g) -w /xmath_walkthrough -v $(pwd):/xmath_walkthrough ghcr.io/xmos/xcore_builder:latest bash -l tools/ci/build_xmath_walkthrough_zip.sh

# get the repo and dependencies
west init -m https://github.com/xmos/xmath_walkthrough/
west update

# make zip archive
zip -r --symlinks xmath_walkthrough xmath_walkthrough

# cleanup
rm -rf xmath_walkthrough
rm -rf .west
