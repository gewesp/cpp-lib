#! /bin/bash

# Exit if any subprogram exits with an error
set -e

if [ "" != "$CMAKE_BUILD_TYPE" ] ; then
  build_type="-D CMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE"
else
  build_type=""
fi

rm -rf build CMakeCache.txt CMakeFiles/
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=clang++ $build_type ..
make -j$(nproc)
