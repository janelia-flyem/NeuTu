#!/bin/bash

uncompress_lib () {
  if [ ! -f $1.tar ]
  then
    gunzip < $1.tar.gz > $1.tar
  fi
  tar -xvf $1.tar
}

libdir=`pwd`
export CFLAGS="-fPIC"

if [ ! -f fftw3/lib/libfftw3.a ]
then
  if [ ! -d fftw3 ]
  then
    mkdir fftw3
  fi

  echo 'Building libfftw3 ...'
  uncompress_lib fftw-3.3.2
  cd fftw-3.3.2
  ./configure --enable-shared=no --with-pic --prefix=${libdir}/fftw3
  make
  make install

  ./configure --enable-shared=no --enable-float --with-pic --prefix=${libdir}/fftw3
  make
  make install
  cd ..
fi

if [ ! -f jansson/lib/libjansson.a ]
then
  echo 'Building libjansson ...'
  if [ ! -d jansson ]
  then
    mkdir jansson
  fi
  uncompress_lib jansson-2.5
  cd jansson-2.5
  ./configure --enable-shared=no --with-pic --prefix=${libdir}/jansson
  make
  make install
  cd ..
fi


if [ ! -f xml/lib/libxml2.a ]
then
  echo 'Building libxml ...'
  if [ ! -d xml ]
  then
    mkdir xml
  fi
  uncompress_lib libxml2-2.9.1
  cd libxml2-2.9.1
  ./configure --without-iconv --without-zlib --without-lzma --with-pic --enable-shared=no --prefix=${libdir}/xml --without-python
  make
  make install
  cd ..
fi

use_png=0
if [ $use_png -ne 0 ]
then
  if [ ! -f png/lib/libpng.a ]
  then
    echo 'Building libpng ...'
    if [ ! -d png ]
    then
      mkdir png
    fi
    uncompress_lib libpng-1.6.7
    cd libpng-1.6.7
    ./configure --enable-shared=no --with-pic --prefix=${libdir}/png
    make
    make install
    cd ..
  fi
fi

if [ ! -f hdf5/lib/libhdf5.a ]
then
  echo 'Building libpng ...'
  if [ ! -d hdf5 ]
  then
    mkdir hdf5
  fi
  uncompress_lib hdf5-1.8.12
  cd hdf5-1.8.12
  ./configure --enable-shared=no --with-pic --prefix=${libdir}/hdf5
  make
  make install
  cd ..
fi

