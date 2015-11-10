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

  condaEnv=$install_dir/Download/miniconda/envs/dvidenv
  if [ `uname` == 'Darwin' ]; then
    QMAKE_SPEC=$install_dir/Trolltech/Qt$qtver/mkspecs/macx-g++
  else
    QMAKE_SPEC=$install_dir/Trolltech/Qt$qtver/mkspecs/linux-g++
  fi
  if [ -d $install_dir/Download/miniconda/envs/dvidenv/include ]
  then
    sh build.sh $install_dir/Trolltech/Qt$qtver/bin/qmake $QMAKE_SPEC -e flyem -q CONDA_ENV=$condaEnv -c $debug_config
  else
    sh build.sh $install_dir/Trolltech/Qt$qtver/bin/qmake $QMAKE_SPEC -e flyem -q BUILDEM_DIR=$install_dir/Download/buildem -c $debug_config
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



