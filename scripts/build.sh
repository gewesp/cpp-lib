#! /bin/bash

# Exit if any subprogram exits with an error
set -e

if [ "" = "$1" ] ; then
  build_type=RelWithDebInfo
else
  build_type="$1"
fi

rm -rf build CMakeCache.txt CMakeFiles/
mkdir build
cd build
echo "Configuring for $build_type ..."
cmake -D CMAKE_CXX_COMPILER=clang++-7 -D CMAKE_BUILD_TYPE=$build_type ..
make -j6
