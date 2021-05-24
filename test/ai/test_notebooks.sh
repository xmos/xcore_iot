#!/bin/bash
set -e

JUNIT_OUTPUT="notebooks_junit.xml"

declare -a jupyter_notebooks=(
    "../examples/bare-metal/cifar10/train/training_and_converting.ipynb"
    )

for notebook in ${jupyter_notebooks[@]}; do
    echo '***************************'
    echo '* Testing' ${notebook}
    echo '***************************'
    (pytest --nbval-lax -v --junitxml ${JUNIT_OUTPUT} ${notebook})
done
