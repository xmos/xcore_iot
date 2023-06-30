#!/bin/bash
set -e
set -x

XCORE_VOICE_ROOT=`git rev-parse --show-toplevel`

source tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_VOICE_ROOT}/dist_pdfs
mkdir -p ${DIST_DIR}

# set doc_builder version
DOC_BUILDER=ghcr.io/xmos/doc_builder:v3.0.0

echo '******************************************************'
echo '* Building PDFs for xmath_walkthrough'
echo '******************************************************'
# xmath_walkthrough is non standard because it is a stand-alone project that we redistribute with the xcore_iot project
full_path="${XCORE_VOICE_ROOT}/xmath_walkthrough"
# build docs
(cd ${full_path}; docker run --rm -t -u "$(id -u):$(id -g)" -v $(pwd):/build -e PDF=1 -e SKIP_LINK=1 -e REPO:/build ${DOC_BUILDER})
# copy to dist folder
(cd ${full_path}/doc/_build/pdf; cp tutorial.pdf ${DIST_DIR}/xmath_walkthrough_tutorial.pdf)
