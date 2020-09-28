#!/bin/bash
set -e

LOG_FILE="$(mktemp)"
echo "Log file: ${LOG_FILE}"

# Run firmware for 10 seconds
timeout 10 xrun --io ../bin/hello_world.xe 2>&1 | head > ${LOG_FILE}

if ! grep -q 'x_value:.*y_value:' ${LOG_FILE}; then
  echo "ERROR: Expected logs not found in output"
  exit 1
fi

echo
echo "~~~ALL TESTS PASSED~~~"