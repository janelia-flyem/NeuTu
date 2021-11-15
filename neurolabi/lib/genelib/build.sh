if [[ $(uname) == 'Darwin' ]]; then
    CC=clang
else
    CC=gcc
fi

CMAKE_GENERATOR=${CMAKE_GENERATOR-Unix Makefiles}
CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE-Release}

export MACOSX_DEPLOYMENT_TARGET=10.9

BUILD_DIR=build
PREFIX=/Users/zhaot/Work/devenv/miniconda/envs/neulib
mkdir -p "${BUILD_DIR}" # Using -p here is convenient for calling this script outside of conda.
cd "${BUILD_DIR}"
cmake ..\
    -G "${CMAKE_GENERATOR}" \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
    -DCMAKE_PREFIX_PATH="${PREFIX}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}" \
    -DCMAKE_SHARED_LINKER_FLAGS="-Wl,-rpath,${PREFIX}/lib -L${PREFIX}/lib" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,${PREFIX}/lib -L${PREFIX}/lib" \
    -DCMAKE_MACOSX_RPATH=ON

make -j2
make install
