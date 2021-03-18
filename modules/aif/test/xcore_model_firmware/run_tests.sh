#!/usr/bin/bash
set -e

declare -a models=(
    "cifar10"
    "vww_96"
)

declare -a num_threads=(
    "1"
    "5"
)

mkdir -p build

for model in ${models[@]}; do
  for par in ${num_threads[@]}; do
    echo "****************************"
    echo "* Building: model=${model}, par=${par}"
    echo "****************************"
    (xformer.py --analyze -par ${par} models/${model}_quant.tflite models/${model}_xcore_par${par}.tflite 2> /dev/null)
    (python ../../tools/generate/generate_model_runner.py --input models/${model}_xcore_par${par}.tflite --output src --name test)
    (cd build; cmake ../; make install)
    (xrun --xscope bin/xcore_model_firmware.xe)
  done
done
