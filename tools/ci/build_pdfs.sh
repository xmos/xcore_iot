#!/bin/bash
set -e

# help text
help()
{
   echo "Module PDF builder script"
   echo
   echo "Syntax: build_pdfs.sh"
   echo
   echo "options:"
   echo "h     Print this Help."
}

# flag arguments
while getopts h option
do
    case "${option}" in
        h) help
           exit;;
    esac
done

XCORE_VOICE_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_VOICE_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_VOICE_ROOT}/dist_pdfs
mkdir -p ${DIST_DIR}

# setup configurations
# row format is: "module_path  generate exclude_patterns generated_filename   final_filename"
standard_modules=(
    "modules/io/modules/mic_array   yes programming_guide.pdf   doc_excludes.txt   mic_array_programming_guide.pdf"
    "modules/io   yes programming_guide.pdf   exclude_patterns.inc   peripheral_io_programming_guide.pdf"
    "modules/rtos   yes programming_guide.pdf   exclude_patterns.inc   rtos_programming_guide.pdf"
    "modules/rtos   no build_system_guide.pdf   exclude_patterns.inc   build_system_guide.pdf"
)

# *****************************************************************
# NOTE: some modules are not standard and are built individually
# *****************************************************************

# perform builds on standard modules
for ((i = 0; i < ${#standard_modules[@]}; i += 1)); do
    read -ra FIELDS <<< ${standard_modules[i]}
    rel_path="${FIELDS[0]}"
    gen_flag="${FIELDS[1]}"
    gen_name="${FIELDS[2]}"
    expat_file="${FIELDS[3]}"
    fin_name="${FIELDS[4]}"
    full_path="${XCORE_VOICE_ROOT}/${rel_path}"

    if [[ ${gen_flag} = "yes" ]] ; then
        echo '******************************************************'
        echo '* Building PDFs for' ${rel_path}
        echo '******************************************************'

        # build docs
        (cd ${full_path}; docker run --rm -t -u "$(id -u):$(id -g)" -v $(pwd):/build -e PDF=1 -e REPO:/build -e DOXYGEN_INCLUDE=/build/doc/Doxyfile.inc -e EXCLUDE_PATTERNS=/build/doc/${expat_file} -e DOXYGEN_INPUT=ignore ghcr.io/xmos/doc_builder:v2.0.0)
    fi

    # copy to dist folder
    (cd ${full_path}/doc/_build/pdf; cp ${gen_name} ${DIST_DIR}/${fin_name})
done

# perform builds on non-standard modules

echo '******************************************************'
echo '* Building PDFs for lib_xcore_math'
echo '******************************************************'
# lib_xcore_math is non standard because the doc_builder returns a non-zero return code but does not generate an error.  
#  To workaround this, the call to docker is redirected so the return code can be ignored.  
full_path="${XCORE_VOICE_ROOT}/modules/core/modules/xcore_math/lib_xcore_math"

# build docs
(cd ${full_path}; docker run --rm -t -u "$(id -u):$(id -g)" -v $(pwd):/build -e PDF=1 -e REPO:/build -e DOXYGEN_INCLUDE=/build/doc/Doxyfile.inc -e EXCLUDE_PATTERNS=/build/doc/doc_excludes.txt -e DOXYGEN_INPUT=ignore ghcr.io/xmos/doc_builder:v2.0.0 || echo "Container always falsely reports an error. Ignoring error.")

# copy to dist folder
(cd ${full_path}/doc/_build/pdf; cp programming_guide.pdf ${DIST_DIR}/xcore_math_programming_guide.pdf)
