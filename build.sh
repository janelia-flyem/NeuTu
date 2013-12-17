#!/bin/bash

#Example:
#sh build.sh /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/bin/qmake /Users/zhaot/local/lib/Trolltech/Qt-4.8.5/mkspecs/macx-g++

if [ $# -lt 2 ]
then
  echo "Usage: sh build.sh <qmake_path> <qmake_spec_path>"
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

./update_library --release 
if [ ! -d build ]
then
  mkdir build
fi

cd build
$QMAKE -spec $QMAKE_SPEC CONFIG+=release CONFIG+=x86_64 -o Makefile ../gui/gui.pro
make

if [ -d neuTube.app ]
then
  cd ..
  sh gui/deploy_neutube.sh `pwd`/build
  cp -r build/neuTube.app ../
else
  mv neuTube ../../
fi

