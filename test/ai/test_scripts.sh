#!/bin/bash
set -e

# test that all scripts can be run from the root path
#   this ensure they have been installed
echo '******************************************************'
echo '* Checking generate_model_runner.py'
echo '******************************************************'
generate_model_runner.py --help

echo '******************************************************'
echo '* Checking convert_tflite_to_c_source.py'
echo '******************************************************'
convert_tflite_to_c_source.py --help

echo '******************************************************'
echo '* Checking xformer.py'
echo '******************************************************'
xformer.py --help


