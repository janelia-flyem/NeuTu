#!/bin/bash
 
make

if [ ! -d build ]
then
  mkdir build
fi

cd build
if [ $1 == debug ]
then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
else
  cmake ..
fi

THREAD_COUNT=${CPU_COUNT:-3}  # conda-build provides CPU_COUNT
make -j${THREAD_COUNT} 
