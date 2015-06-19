#!/bin/bash

#Example:
#sh build_debug.sh /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/bin/qmake /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++

echo 'Obsolete script. Abort.'
exit

if [ $# -lt 2 ]
then
  echo "Usage: sh build_debug.sh <qmake_path> <qmake_spec_path>"
  echo "Example: "
  echo 'sh build.sh $HOME/local/lib/Trolltech/Qt-4.8.5/bin/qmake $HOME/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++'
  exit 1
fi

QMAKE=$1
QMAKE_SPEC=$2

cd neurolabi

echo 'Building 3rd-party libraries ...'
cd lib
sh build.sh
cd ..

echo 'Building libneurolabi ...'
./update_library 

if [ ! -d build_debug ]
then
  mkdir build_debug
fi

cd build_debug
$QMAKE -spec $QMAKE_SPEC CONFIG+=debug CONFIG+=x86_64 -o Makefile ../gui/gui.pro
make -j3
