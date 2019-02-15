#!/bin/bash

#Example:
#sh build.sh /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/bin/qmake /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++

echo "Build args: $*"
config_options="debug|force_debug_info|release|sanitize"

if [ $# -lt 1 ]
then
  echo "Usage: sh build.sh <qmake_path> <qmake_spec_path> [-d cxx_define] [-e edition] [-c $config_options]"
  echo "Usage: sh build.sh <qt_dir> [-d cxx_define] [-e edition] [-c $config_options]"
  echo "Example: "
  echo 'sh build.sh $HOME/local/lib/Trolltech/Qt-4.8.5/bin/qmake $HOME/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++'
  exit 1
fi

echo $1 |grep '/qmake$'

if [ $? -eq 0 ]
then
  QMAKE=$1
  shift
  if [ $# -lt 1 ]
  then
    echo "Usage: sh build.sh <qmake_path> <qmake_spec_path> [-d cxx_define] [-e edition] [-c $config_options] [-q qmake_flags] [-m make_flags]"
    exit 1
  fi
  QMAKE_SPEC=$1
  shift
else
  QMAKE=$1/bin/qmake
  if [ `uname` = 'Darwin' ]; then
    if [ -n "$edition" ]
    then
      if [ $edition = "flyem" ] || [ $edition = "neu3" ]
      then
        QMAKE_SPEC=$1/mkspecs/macx-clang
      else
        QMAKE_SPEC=$1/mkspecs/macx-g++
      fi
    else
      QMAKE_SPEC=$1/mkspecs/macx-g++
    fi
  else
    QMAKE_SPEC=$1/mkspecs/linux-g++
  fi
  shift
fi

echo $QMAKE
echo $QMAKE_SPEC

set -e

edition=general
debug_config=release
make_args='-j3'
while getopts d:e:c:q:m: option
do
  echo $option
  echo $OPTARG
  case $option in
    d)
      cxx_define=$OPTARG;;
    e)
      edition=$OPTARG;;
    c)
      debug_config=$OPTARG;;
    q)
      ext_qmake_args=$OPTARG;;
    m)
      make_args=$OPTARG;;
  esac
done

if [ -n "$cxx_define" ]
then
  if [ $edition = "biocytin" ]
  then
    cxx_define="_BIOCYTIN_ $cxx_define"
  fi
else
  if [ $edition = "biocytin" ]
  then
    cxx_define="_BIOCYTIN_"
  fi
fi

if [ ! -z "$QMAKE_SPEC" ]
then
  qmake_args="-spec $QMAKE_SPEC"
fi

if [ ! -z "$CONDA_ENV" ]
then
  qmake_args="$qmake_args 'CONDA_ENV=${CONDA_ENV}'"
fi

if [ -n "$edition" ]
then
  if [ $edition = "flyem" ]
  then
    qmake_args="$qmake_args CONFIG+=flyem"
  fi

  if [ $edition = "neu3" ]
  then
    qmake_args="$qmake_args CONFIG+=neu3"
  fi
fi

qmake_args="$qmake_args CONFIG+=$debug_config CONFIG+=x86_64 -o Makefile ../gui/gui.pro"

if [ $debug_config = "sanitize" ]
then
  qmake_args="$qmake_args CONFIG+=debug"
fi

if [ $debug_config = "debug" ]
then
  qmake_args="$qmake_args CONFIG+=release"
fi

if [ -n "$cxx_define" ]
then
  qmake_args="$qmake_args DEFINES+=\"$cxx_define\""
fi

if [ -n "$PKG_VERSION" ]
then
  qmake_args="$qmake_args DEFINES+=_PKG_VERSION=\"$PKG_VERSION\""
fi

if [ -n "$ext_qmake_args" ]
then
  ext_qmake_args=`echo "$ext_qmake_args" | sed -e 's/^"//' -e 's/"$//'`
  echo $ext_qmake_args
  qmake_args="$qmake_args $ext_qmake_args"
fi

#echo $qmake_args
#exit

cd neurolabi

if [ -z "$CONDA_ENV" ]
then
  echo 'Building 3rd-party libraries ...'
  cd lib
  sh build.sh
  cd ..
fi

echo 'Building libneurolabi ...'
build_dir=build
if [ $debug_config = "debug" ]
then
  ./update_library
  build_dir=build_debug
else
  if [ $debug_config = "sanitize" ]
  then
    ./update_library --sanitize
    build_dir=build_sanitize
  else
    ./update_library --release 
  fi
fi

if [ ! -d $build_dir ]
then
  mkdir $build_dir
fi

cd $build_dir
echo "qmake_args: $qmake_args"
echo $qmake_args > source.qmake
echo $qmake_args | xargs $QMAKE
echo "qmake done"


THREAD_COUNT=${CPU_COUNT:-3}  # conda-build provides CPU_COUNT
make -j${THREAD_COUNT}

bin_dir=.
app_name=neuTube

if [ $edition = "flyem" ]
then
  app_name=neutu
fi

if [ $edition = "neu3" ]
then
  app_name=neu3
fi

if [ $debug_config = "debug" ]
then
  app_name=${app_name}_d
fi

if [ -d $bin_dir/$app_name.app ]
then
  bin_dir=$bin_dir/$app_name.app/Contents/MacOS
fi

if [ ! -d $bin_dir/doc ]
then
  cp -r ../gui/doc $bin_dir/
fi

if [ $edition = "flyem" ] || [ $edition = "neu3" ]
then
  cp ../gui/config_flyem.xml $bin_dir/config.xml
  cp ../gui/doc/flyem_proofread_help.html $bin_dir/doc/shortcut.html
  cp -r ../json $bin_dir
fi

if [ $edition = "biocytin" ]
then
  cp ../gui/biocytin_config.xml $bin_dir/config.xml
fi

if [ $edition = "general" ]
then
  cp ../gui/config.xml $bin_dir/config.xml
fi
#echo "Deploying ..."

#if [ -d neuTube.app ]
#then
#  qtbin_dir=`dirname $QMAKE`
#  cd ..
#  if [ -f $qtbin_dir/macdeployqt ]
#  then
#    $qtbin_dir/macdeployqt `pwd`/build/neuTube.app -no-plugins
#  else
#    sh gui/deploy_neutube.sh `pwd`/build
#  fi
#  echo "build/neuTube.app deployed."
#else
#  echo "neuTube deployed."
  #mv neuTube ../../
#fi

