#!/bin/bash

#Example:
#sh build.sh /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/bin/qmake /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++

if [ $# -lt 2 ]
then
  echo "Usage: sh build.sh <qmake_path> <qmake_spec_path> [-d cxx_define] [-e edition] [-c debug|release]"
  echo "Example: "
  echo 'sh build.sh $HOME/local/lib/Trolltech/Qt-4.8.5/bin/qmake $HOME/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++'
  exit 1
fi

QMAKE=$1
QMAKE_SPEC=$2
shift
shift

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
  if [ $edition = "flyem" ]
  then
    cxx_define="_FLYEM_ $cxx_define"
  fi

  if [ $edition = "biocytin" ]
  then
    cxx_define="_BIOCYTIN_ $cxx_define"
  fi
else
  if [ $edition = "flyem" ]
  then
    cxx_define="_FLYEM_"
  fi

  if [ $edition = "biocytin" ]
  then
    cxx_define="_BIOCYTIN_"
  fi
fi

qmake_args="-spec $QMAKE_SPEC CONFIG+=$debug_config CONFIG+=x86_64 -o Makefile ../gui/gui.pro"
if [ -n "$cxx_define" ]
then
  qmake_args="$qmake_args DEFINES+=\"$cxx_define\""
fi

if [ -n "$ext_qmake_args" ]
then
  echo $ext_qmake_args
  qmake_args="$qmake_args $ext_qmake_args"
fi

#exit 1

echo $qmake_args

cd neurolabi

echo 'Building 3rd-party libraries ...'
cd lib
sh build.sh
cd ..

echo 'Building libneurolabi ...'
build_dir=build
if [ $debug_config = "debug" ]
then
  ./update_library
  build_dir=build_debug
else
  ./update_library --release 
fi

if [ ! -d $build_dir ]
then
  mkdir $build_dir
fi

cd $build_dir
echo $qmake_args > source.qmake
$QMAKE $qmake_args 
make -j3

if [ $edition = "flyem" ]
then
  cp ../gui/config_flyem.xml config.xml
  cp -r ../json .
fi

if [ $edition = "biocytin" ]
then
  cp ../gui/biocytin_config.xml config.xml
fi

if [ $edition = "general" ]
then
  cp ../gui/config.xml config.xml
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

