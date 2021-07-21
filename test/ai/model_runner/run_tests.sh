#!/bin/bash
set -e

REPORT=test.rpt
FIRMWARE=model_runner.xe
SPLITFILE=model_runner.xb

rm -f ${REPORT}

declare -a models=(
    "cifar10"
    "vww_96"
)

declare -a num_threads=(
    "1"
    "5"
)

for model in ${models[@]}; do
  for par in ${num_threads[@]}; do
    QUANT_MODEL="models/${model}_quant.tflite"
    XCORE_MODEL="models/${model}_xcore_par${par}.tflite"

    echo "****************************"  | tee -a ${REPORT}
    echo "****************************"  | tee -a ${REPORT}
    echo "****************************"  | tee -a ${REPORT}
    echo "START_TEST"  | tee -a ${REPORT}
    echo "model=${model}"  | tee -a ${REPORT}
    echo "par=${par}"  | tee -a ${REPORT}
    echo "ARENA_SIZE"  | tee -a ${REPORT}
    (xformer.py --analyze -par ${par} ${QUANT_MODEL} ${XCORE_MODEL} 2> /dev/null | tee -a ${REPORT})
    (python ${XCORE_SDK_PATH}/modules/aif/tools/generate/generate_model_runner.py --input ${XCORE_MODEL} --output src --name test)

    (rm -rf build; cmake -B build; cmake --build build --target install)
    echo "CODE_SIZE"  | tee -a ${REPORT}
    (xobjdump --size bin/${FIRMWARE} | tee -a ${REPORT})

    echo "SRAM_DURATION"  | tee -a ${REPORT}
    (xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT})

    (rm -rf build; cmake -B build -DUSE_EXTMEM=1; cmake --build build --target install)
    echo "EXTMEM_DURATION"  | tee -a ${REPORT}
    (xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT})

    (rm -rf build; cmake -B build -DUSE_SWMEM=1; cmake --build build --target install)
    (cd bin; xobjdump --strip ${FIRMWARE}; xobjdump --split ${SPLITFILE}; xflash --write-all image_n0c0.swmem --target XCORE-AI-EXPLORER)
    echo "SWMEM_DURATION"  | tee -a ${REPORT}
    (xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT})
    echo "END_TEST"  | tee -a ${REPORT}
  done
done
