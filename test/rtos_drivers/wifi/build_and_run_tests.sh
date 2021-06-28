#!/bin/bash
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

echo "****************"
echo "* Build        *"
echo "****************"
make distclean
make -j

./run_tests.sh
