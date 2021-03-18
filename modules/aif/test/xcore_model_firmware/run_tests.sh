#!/usr/bin/bash
set -e

REPORT=test.rpt
FIRMWARE=bin/xcore_model_firmware.xe

rm -f ${REPORT}

declare -a models=(
    "cifar10"
    "vww_96"
)

declare -a num_threads=(
    "1"
    "5"
)

rm -rf build
mkdir -p build

for model in ${models[@]}; do
  for par in ${num_threads[@]}; do
    QUANT_MODEL="models/${model}_quant.tflite"
    XCORE_MODEL="models/${model}_xcore_par${par}.tflite"

    echo "****************************"  | tee -a ${REPORT}
    echo "****************************"  | tee -a ${REPORT}
    echo "****************************"  | tee -a ${REPORT}
    echo "TEST"  | tee -a ${REPORT}
    echo "model=${model}"  | tee -a ${REPORT}
    echo "par=${par}"  | tee -a ${REPORT}
    echo "ARENA_SIZE"  | tee -a ${REPORT}
    (xformer.py --analyze -par ${par} ${QUANT_MODEL} ${XCORE_MODEL} 2> /dev/null | tee -a ${REPORT})
    (python ../../tools/generate/generate_model_runner.py --input ${XCORE_MODEL} --output src --name test)
    (cd build; rm -rf *; cmake ../; make install)
    echo "CODE_SIZE"  | tee -a ${REPORT}
    (xobjdump --size ${FIRMWARE} | tee -a ${REPORT})
    echo "SRAM_DURATION"  | tee -a ${REPORT}
    (xrun --xscope ${FIRMWARE} 2>&1 | tee -a ${REPORT})
    (cd build; rm -rf *; cmake ../ -DUSE_EXTMEM=1; make clean install)
    echo "EXTMEM_DURATION"  | tee -a ${REPORT}
    (xrun --xscope ${FIRMWARE} 2>&1 | tee -a ${REPORT})
  done
done
