#!/usr/bin/env bash

# set configuration vars

[[ -z $1 ]] && echo "error: source directory was not provided" && exit 1

SRC_DIR=$1
INSTALL_DIR=${2:-../install/}

# print configuration vars

echo "info: printing configuration vars"
echo "info: source dir: ${SRC_DIR}"
echo "info: install dir: ${INSTALL_DIR}"
echo ""


cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_CXX_FLAGS="${CXX_FLAGS}" \
  -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_SHARED_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_MODULE_LINKER_FLAGS="${LINKER_FLAGS}" \
  -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=On \
  -DPRJ_USE_LLVM_INTERNAL_MODULE="${PRJ_USE_LLVM_INTERNAL_MODULE}" \
  -DPRJ_SKIP_TESTS="${PRJ_SKIP_TESTS}" \
  -DPRJ_DEBUG="${PRJ_DEBUG}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  ${CMAKE_OPTIONS} \
  "${SRC_DIR}"

exit $?
