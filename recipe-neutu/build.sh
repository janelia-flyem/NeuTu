additional_qflag='CONFIG+=NO_CONDA_LIB_CHECK'
if [ $(uname) == 'Darwin' ]; then
    CC=/usr/bin/cc
    CXX=/usr/bin/clang
else
    # conda is providing gcc and defining $CC,
    # but the binary isn't named 'gcc'.
    # Create a symlink for build scripts that expect that name.
    cd $(dirname ${CC}) && ln -s $(basename ${CC}) gcc && cd -
    cd $(dirname ${CXX}) && ln -s $(basename ${CXX}) g++ && cd -
    cd $(dirname ${LD}) && ln -s $(basename ${LD}) ld && cd -
    additional_qflag="$additional_qflag LIBS+=-Wl,-rpath-link,/usr/lib64 LIBS+=-Wl,-rpath-link,/lib64 LIBS+=-L/usr/lib64 INCLUDEPATH+=/usr/include"
fi

if [ $(uname) == 'Darwin' ]; then
    QMAKE_SPEC_PATH=${PREFIX}/mkspecs/macx-clang
else
    QMAKE_SPEC_PATH=${PREFIX}/mkspecs/linux-g++-64
fi

export CONDA_ENV=${PREFIX}

app_name=${NEUTU_TARGET:-neutu}
app_base_name=$app_name
if [ ${NEUTU_TARGET} == 'neutu-debug' ]
then
  app_base_name=neutu
  app_name=${app_base_name}_d
fi

if [ ${NEUTU_TARGET} == 'neu3-debug' ]
then
  app_base_name=neu3
  app_name=${app_base_name}_d
fi

for x in neutu-develop neutu-alpha neutu-beta neutu neutu-di
do
  if [ ${NEUTU_TARGET} == "$x" ]
  then
    app_base_name=neutu
    app_name=$app_base_name
    break
  fi
done

for x in neu3-develop neu3-alpha neu3-beta neu3 neu3-di
do
  if [ ${NEUTU_TARGET} == "$x" ]
  then
    app_base_name=neu3
    app_name=$app_base_name
    break
  fi
done
echo "app name: $app_name"

for x in neutube-develop neutube-alpha neutube-beta neutube neutube-di
do
  if [ ${NEUTU_TARGET} == "$x" ]
  then
    app_base_name=neuTube
    app_name=$app_base_name
    break
  fi
done

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

if [ "${NEUTU_TARGET}" == 'neutu-di' ] || [ "${NEUTU_TARGET}" == 'neu3-di' ]
then
  build_flag='-c force_debug_info'
fi

edition=flyem
if [ "$app_base_name" == 'neu3' ]
then
  edition=neu3
elif [ "$app_base_name" == 'neuTube' ]
then
  edition=neutube
fi

#if [ ! -z "$additional_qflag" ]
#then
#  build_flag="$build_flag -q $additional_qflag"
#fi

echo "Build flag: $build_flag"
bash -x -e build.sh ${PREFIX}/bin/qmake ${QMAKE_SPEC_PATH} -e $edition $build_flag -q "$additional_qflag"

# Install to conda environment
if [ $(uname) == 'Darwin' ]; then
    mv ${build_dir}/${app_name}.app ${PREFIX}/bin/
else
    mv ${build_dir}/${app_name} ${PREFIX}/bin/
    mv ${build_dir}/${app_base_name}_config ${PREFIX}/bin/
fi
