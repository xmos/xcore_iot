#!/bin/bash
set -e

JUNIT_OUTPUT="notebooks_junit.xml"

declare -a jupyter_notebooks=(
    "../documents/reference/ai_tools/training_and_converting.ipynb"
    )

for notebook in ${jupyter_notebooks[@]}; do
    echo '***************************'
    echo '* Testing' ${notebook}
    echo '***************************'
    (pytest --nbval-lax -v --junitxml ${JUNIT_OUTPUT} ${notebook})
done
