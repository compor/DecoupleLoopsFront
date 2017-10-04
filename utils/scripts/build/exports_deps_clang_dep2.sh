#!/usr/bin/env bash

export CC=clang 
export CXX=clang++

export LLVMCONFIG=$(which llvm-config)

export LLVM_DIR=$(${LLVMCONFIG} --prefix)/share/llvm/cmake/

export GTEST_ROOT=/usr/local/gtest-libcxx

export BOOST_ROOT=/bulk/workbench/boost/015900/install/

export CXX_FLAGS="-stdlib=libc++"

export LINKER_FLAGS="-Wl,-L$(${LLVMCONFIG} --libdir)" 
export LINKER_FLAGS="${LINKER_FLAGS} -lc++ -lc++abi" 

export BUILD_TYPE=Debug

export PRJ_USE_LLVM_INTERNAL_MODULE=OFF
export PRJ_SKIP_TESTS=OFF
export PRJ_DEBUG=ON

[[ -z ${DECOUPLELOOPS_ROOT} ]] && echo "missing DECOUPLELOOPS_ROOT"
[[ -z ${AnnotateLoops_DIR} ]] && echo "missing AnnotateLoops_DIR"
export DECOUPLELOOPS_ROOT
export AnnotateLoops_DIR


CMAKE_OPTIONS="-DLLVM_DIR=${LLVM_DIR}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DDECOUPLELOOPS_ROOT=${DECOUPLELOOPS_ROOT}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DAnnotateLoops_DIR=${AnnotateLoops_DIR}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBOOST_ROOT=${BOOST_ROOT}"

export CMAKE_OPTIONS

