#!/bin/bash

#Example:
#sh build.sh /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/bin/qmake /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++

if [ $# -lt 2 ]
then
  echo "Usage: sh build.sh <qmake_path> <qmake_spec_path> [-d cxx_define -e edition]"
  echo "Example: "
  echo 'sh build.sh $HOME/local/lib/Trolltech/Qt-4.8.5/bin/qmake $HOME/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++'
  exit 1
fi

QMAKE=$1
QMAKE_SPEC=$2
shift
shift

edition=general
while getopts d:e: option
do
  echo $option
  case $option in
    d)
      cxx_define=$OPTARG;;
    e)
      edition=$OPTARG;;
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

qmake_args="-spec $QMAKE_SPEC CONFIG+=release CONFIG+=x86_64 -o Makefile ../gui/gui.pro"
if [ -n "$cxx_define" ]
then
  qmake_args="$qmake_args DEFINES+=\"$cxx_define\""
fi

#exit 1

echo $qmake_args

cd neurolabi

echo 'Building 3rd-party libraries ...'
cd lib
sh build.sh
cd ..

echo 'Building libneurolabi ...'
./update_library --release 

if [ ! -d build ]
then
  mkdir build
fi

cd build
echo $qmake_args > source.qmake
$QMAKE $qmake_args 
make

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

