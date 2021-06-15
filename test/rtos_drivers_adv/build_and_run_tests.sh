#!/bin/bash
set -e

echo "****************"
echo "* Build        *"
echo "****************"
make distclean
make -j

./run_tests.sh
