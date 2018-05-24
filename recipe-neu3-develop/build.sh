if [ $(uname) == 'Darwin' ]; then
    CC=/usr/bin/cc
    CXX=/usr/bin/clang
fi

if [ $(uname) == 'Darwin' ]; then
    QMAKE_SPEC_PATH=${PREFIX}/mkspecs/macx-clang
else
    QMAKE_SPEC_PATH=${PREFIX}/mkspecs/linux-g++-64
fi

export CONDA_ENV=${PREFIX}

bash -x -e build.sh ${PREFIX}/bin/qmake ${QMAKE_SPEC_PATH} -e neu3

# Install to conda environment
if [ $(uname) == 'Darwin' ]; then
    mv neurolabi/build/neu3.app ${PREFIX}/bin/
else
    mv neurolabi/build/neu3 ${PREFIX}/bin/
    mv neurolabi/build/config.xml ${PREFIX}/bin/
    mv neurolabi/build/doc ${PREFIX}/bin/
    mv neurolabi/build/json ${PREFIX}/bin/
fi
