#!/usr/bin/env bash

export CC=clang 
export CXX=clang++

export LLVMCONFIG=$(which llvm-config)

export LLVM_DIR=$(${LLVMCONFIG} --prefix)/share/llvm/cmake/

${LLVMCONFIG} --cmakedir &> /dev/null
[[ $? -eq 0 ]] && export LLVM_DIR=$(${LLVMCONFIG} --cmakedir)

export GTEST_ROOT=/usr/local/gtest-libcxx

export BOOST_ROOT=/bulk/workbench/boost/015900/install/

export CXX_FLAGS="-stdlib=libc++"

export LINKER_FLAGS="-Wl,-L$(${LLVMCONFIG} --libdir)" 
export LINKER_FLAGS="${LINKER_FLAGS} -lc++ -lc++abi" 

export BUILD_TYPE=Debug

export DECOUPLELOOPSFRONT_SKIP_TESTS=OFF
export DECOUPLELOOPSFRONT_DEBUG=ON


CMAKE_OPTIONS="-DLLVM_DIR=${LLVM_DIR}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DBOOST_ROOT=${BOOST_ROOT}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DGTEST_ROOT=${GTEST_ROOT}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DDECOUPLELOOPSFRONT_SKIP_TESTS=${DECOUPLELOOPSFRONT__SKIP_TESTS}"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DDECOUPLELOOPSFRONT_DEBUG=${DECOUPLELOOPSFRONT__DEBUG}"

export CMAKE_OPTIONS

