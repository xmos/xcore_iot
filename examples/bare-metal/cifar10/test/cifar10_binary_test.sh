#!/bin/bash
set -e

LOG_FILE="$(mktemp)"
echo "Log file: ${LOG_FILE}"

# Run firmware for DURATION seconds
xrun --io ../bin/cifar10.xe 2>&1 | tee ${LOG_FILE}

if ! grep 'Classification = Ship' ${LOG_FILE}; then
  echo "ERROR: Expected classification not found in output"
  exit 1
fi

echo
echo "~~~ALL TESTS PASSED~~~"