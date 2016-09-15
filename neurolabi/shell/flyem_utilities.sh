function update_gcc 
{
  condaDir=$1
  CONDA_ROOT=`$condaDir/bin/conda info --root`
  if [ `uname` != 'Darwin' ]
  then
    GCCVER=$(gcc --version | grep ^gcc | sed 's/^.* //g')
    if [ $GCCVER \> '4.9.0' ]
    then
      if [ ! -f $condaDir/envs/dvidenv/bin/gcc ]
      then
        source ${CONDA_ROOT}/bin/activate dvidenv
        $condaDir/bin/conda install -c https://conda.anaconda.org/cgat gcc -y
      fi
    fi

    if [ $GCCVER \< '4.8.0' ]
    then
      if [ ! -f $condaDir/envs/dvidenv/bin/gcc ]
      then
        source ${CONDA_ROOT}/bin/activate dvidenv
        $condaDir/bin/conda install -c https://conda.anaconda.org/cgat gcc -y
      fi
    fi
  fi

}

function flyem_build_lowtis {
  install_dir=$1
  downloadDir=$install_dir/Download
  scriptDir=$2
  condaDir=$downloadDir/miniconda
  envDir=$condaDir/envs/dvidenv

  update_gcc $condaDir
  
  if [ `uname` != 'Darwin' ]
  then
    if [ -d $downloadDir/lowtis ]
    then
      cd $downloadDir/lowtis
      git pull
    else
      git clone https://github.com/janelia-flyem/lowtis.git $downloadDir/lowtis
    fi

    cp $scriptDir/lowtis_cmakelists.txt $downloadDir/lowtis/CMakeLists.txt
    cd $downloadDir/lowtis
    mkdir build
    cd build
    cmake -DCMAKE_PREFIX_PATH=$envDir ..
    make -j3
    cd ..
    mkdir build_debug
    cd build_debug
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$envDir ..
    make -j3
    cd ..
    cp -r lowtis $envDir/include
    cp build/liblowtis.so $envDir/lib/
    cp build_debug/liblowtis-g.so $envDir/lib/
  fi
}

function flyem_neutu_update {
  if [ $# -ge 1 ]
  then
    install_dir=$1
  else
    install_dir=/opt
  fi

  debug_config=release
  if [ $# -ge 2 ]
  then
    debug_config=$2
  fi

  qtver=4.8.5
  if [ `uname` == 'Darwin' ]
  then
    qtver=4.8.4
  fi

  condaDir=$install_dir/Download/miniconda
  condaEnv=$condaDir/envs/dvidenv
  CONDA_ROOT=`$condaDir/bin/conda info --root`
  if [ -d $install_dir/update_dvidcpp ]
  then
    if [ -d $condaEnv ]
    then
      source ${CONDA_ROOT}/bin/activate root
      conda update -y conda
      conda remove -y libdvid-cpp -n dvidenv
      if [ -d $condaEnv/include/libdvid ]
      then
        rm -rf $condaEnv/include/libdvid
      fi
      conda install -y -n dvidenv -c flyem libdvid-cpp
    fi
  fi

  if [ -d $condaEnv/include/lowtis ]
  then
    build_flag="-d _ENABLE_LOWTIS_"
    ext_qt_flag="CONFIG+=c++11"
  fi

  if [ -f $condaEnv/bin/gcc ]
  then
    export PATH=$condaEnv/bin:$PATH
  elif [ -f /opt/gcc482/bin/gcc-4.8.2 ]
  then
    export PATH=/opt/gcc482/bin:$PATH
  fi

  if [ `uname` == 'Darwin' ]; then
    QMAKE_SPEC=$install_dir/Trolltech/Qt$qtver/mkspecs/macx-g++
  else
    QMAKE_SPEC=$install_dir/Trolltech/Qt$qtver/mkspecs/linux-g++
  fi
  if [ -d $install_dir/Download/miniconda/envs/dvidenv/include ]
  then
    sh build.sh $install_dir/Trolltech/Qt$qtver/bin/qmake $QMAKE_SPEC -e flyem -q "\"CONDA_ENV=$condaEnv $ext_qt_flag\"" -c $debug_config $build_flag 
  else
    sh build.sh $install_dir/Trolltech/Qt$qtver/bin/qmake $QMAKE_SPEC -e flyem -q "BUILDEM_DIR=$install_dir/Download/buildem $ext_qt_flag" -c $debug_config $build_flag 
  fi

  build_dir=build
  if [ $debug_config == 'debug' ]
  then
    build_dir=build_debug
  fi

  $install_dir/Download/neutube/neurolabi/shell/flyem_post_install $install_dir/Download/neutube $install_dir/Download/neutube/neurolabi/$build_dir
}

function update_neutu {
  install_dir=$1
  target_dir=$1/Download/neutube
  if [ $# -ge 2 ]
  then
    branch=$2
  fi
  source_dir=/groups/flyem/home/zhaot/Work/neutube_ws

  if [ -z $branch ]
  then
    if [ ! -d $target_dir ]
    then
      /opt/bin/git clone -b $branch $source_dir/.git $target_dir
    else
      cd $target_dir
      /usr/bin/git pull origin $branch
    fi
  else
    if [ ! -d $target_dir ]
    then
      /opt/bin/git clone $source_dir/.git $target_dir
    else
      cd $target_dir
      /usr/bin/git pull
    fi
  fi

  master_dir=$source_dir

  flyem_neutu_update $install_dir
  if [ -d $target_dir/neurolabi/build_debug ]
  then
    flyem_neutu_update $install_dir debug
  fi
}



