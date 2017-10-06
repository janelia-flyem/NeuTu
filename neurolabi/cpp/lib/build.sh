#!/bin/bash
 
make

if [ ! -d build ]
then
  mkdir build
fi

cd build
config=$1
if [ ${config:-release} == debug ]
then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
else
  cmake -DCMAKE_BUILD_TYPE=Release ..
fi

THREAD_COUNT=${CPU_COUNT:-3}  # conda-build provides CPU_COUNT
make -j${THREAD_COUNT} 
