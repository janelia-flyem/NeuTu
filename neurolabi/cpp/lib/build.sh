#!/bin/bash
 
make

if [ ! -d build ]
then
  mkdir build
fi

cd build
cmake ..

THREAD_COUNT=${CPU_COUNT:-3}  # conda-build provides CPU_COUNT
make -j${THREAD_COUNT}
