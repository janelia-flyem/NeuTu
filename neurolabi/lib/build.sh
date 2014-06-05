#!/bin/bash

uncompress_lib () {
  if [ ! -f $1.tar ]
  then
    gunzip < $1.tar.gz > $1.tar
  fi
  tar -xvf $1.tar
}

libdir=`pwd`

if [ ! -f fftw3/lib/libfftw3.a ]
then
  if [ ! -d fftw3 ]
  then
    mkdir fftw3
  fi

  echo 'Building libfftw3 ...'
  uncompress_lib fftw-3.3.2
  #if [ ! -f fftw-3.3.2.tar ]
  #then
  #  gunzip fftw-3.3.2.tar.gz
  #fi
  #tar -xvf fftw-3.3.2.tar
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
  #gunzip jansson-2.5.tar.gz
  #tar -xvf jansson-2.5.tar
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
  #gunzip libxml2-2.9.1.tar.gz
  #tar -xvf libxml2-2.9.1.tar
  cd libxml2-2.9.1
  ./configure --without-iconv --without-zlib --with-pic --enable-shared=no --prefix=${libdir}/xml
  make
  make install
  cd ..
fi

if [ ! -f png/lib/libpng.a ]
then
  echo 'Building libpng ...'
  if [ ! -d png ]
  then
    mkdir png
  fi
  uncompress_lib libpng-1.6.7
  #gunzip libpng-1.6.7.tar.gz
  #tar -xvf libpng-1.6.7.tar
  cd libpng-1.6.7
  ./configure --enable-shared=no --with-pic --prefix=${libdir}/png
  make
  make install
  cd ..
fi

if [ ! -f hdf5/lib/libhdf5.a ]
then
  echo 'Building libpng ...'
  if [ ! -d hdf5 ]
  then
    mkdir hdf5
  fi
  uncompress_lib hdf5-1.8.12.tar.gz
  #gunzip hdf5-1.8.12.tar.gz
  #tar -xvf hdf5-1.8.12.tar
  cd hdf5-1.8.12
  ./configure --enable-shared=no --with-pic --prefix=${libdir}/hdf5
  make
  make install
  cd ..
fi

