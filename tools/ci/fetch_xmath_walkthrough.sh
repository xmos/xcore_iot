#!/bin/bash
set -e
set -x

# get the repo and dependencies
west init -m https://github.com/xmos/xmath_walkthrough/
west update
