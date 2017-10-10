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

app_name=neutu
build_dir=neurolabi/build
if [ "X${NEUTU_BUILD_MODE}" == 'Xdebug' ]
then
  qtlib_dir=${PREFIX}/lib
  cd neurolabi/shell
  ./fixqtdebug Qt5 $qtlib_dir
  build_flag='-c debug'
  app_name=neutu_d
  build_dir=neurolabi/build_debug
  cd ../../
fi

bash -x -e build.sh ${PREFIX}/bin/qmake ${QMAKE_SPEC_PATH} -e flyem $build_flag

# Install to conda environment
if [ $(uname) == 'Darwin' ]; then
    mv ${build_dir}/${app_name}.app ${PREFIX}/bin/
else
    mv ${build_dir}/${app_name} ${PREFIX}/bin/
    mv ${build_dir}/config.xml ${PREFIX}/bin/
    mv ${build_dir}/doc ${PREFIX}/bin/
    mv ${build_dir}/json ${PREFIX}/bin/
fi
