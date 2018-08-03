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

app_name=${NEUTU_TARGET:-neutu}
if [ ${NEUTU_TARGET} == 'neutu-debug' ]
then
  app_name=neutu_d
fi

if [ ${NEUTU_TARGET} == 'neu3-debug' ]
then
  app_name=neu3_d
fi

for x in neutu-develop neutu-alpha neutu-beta neutu
do
  if [ ${NEUTU_TARGET} == "$x" ]
  then
    app_name=neutu
    break
  fi
done

for x in neu3-develop neu3-alpha neu3-beta neu3
do
  if [ ${NEUTU_TARGET} == "$x" ]
  then
    app_name=neu3
    break
  fi
done
echo "app name: $app_name"

build_dir=neurolabi/build
if [ "$app_name" == 'neutu_d' ] || [ "$app_name" == 'neu3_d' ]
then
  qtlib_dir=${PREFIX}/lib
  cd neurolabi/shell
  ./fixqtdebug Qt5 $qtlib_dir
  build_flag='-c debug'
  build_dir=neurolabi/build_debug
  cd ../../
fi

edition=flyem
if [ "$app_name" == 'neu3_d' ] || [ "$app_name" == 'neu3' ]
then
  edition=neu3
fi

bash -x -e build.sh ${PREFIX}/bin/qmake ${QMAKE_SPEC_PATH} -e $edition $build_flag

# Install to conda environment
if [ $(uname) == 'Darwin' ]; then
    mv ${build_dir}/${app_name}.app ${PREFIX}/bin/
else
    mv ${build_dir}/${app_name} ${PREFIX}/bin/
    mv ${build_dir}/config.xml ${PREFIX}/bin/
    mv ${build_dir}/doc ${PREFIX}/bin/
    mv ${build_dir}/json ${PREFIX}/bin/
fi
