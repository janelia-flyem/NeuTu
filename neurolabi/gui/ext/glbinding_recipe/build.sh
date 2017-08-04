#!/bin/bash

mkdir build
cd build

BUILD_CONFIG=Release

# choose different screen settings for OS X and Linux
if [ `uname` = "Darwin" ]; then
    SCREEN_ARGS=(
        "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.9"
    )
else
    SCREEN_ARGS=(
        
    )
fi

# now we can start configuring
cmake .. -G "Ninja" \
    -Wno-dev \
    -DCMAKE_BUILD_TYPE=$BUILD_CONFIG \
    -DCMAKE_PREFIX_PATH:PATH="${PREFIX}" \
    -DCMAKE_INSTALL_PREFIX:PATH="${PREFIX}" \
    -DCMAKE_INSTALL_RPATH:PATH="${PREFIX}/lib" \
    -DOPTION_BUILD_TOOLS:BOOL=OFF \
    -DBUILD_SHARED_LIBS:BOOL=OFF \
    -DOPTION_BUILD_TESTS:BOOL=OFF \
    -DOPTION_BUILD_GPU_TESTS:BOOL=OFF \
    -DCMAKE_CXX_FLAGS:STRING="${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11" \
    ${SCREEN_ARGS[@]}

# compile & install!
ninja install
