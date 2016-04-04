NeuTu (a.k.a. neuTube)
=====

[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)

Software package for neuron reconstruction and visualization

## Download

    git clone -b public https://github.com/janelia-flyem/NeuTu.git NeuTu

## Build

### Linux and Mac

1. Make sure you have installed Qt 4.8.1+ (Qt 4.8.4 recommended)

    Various versions of Qt can be dowloaded from https://download.qt.io/archive/qt/
    
2. Go to the NeuTu directory and run

####

    sh build.sh <qmake_path> <qmake_spec_path>

by specifying the qmake command path and the corresponding spec path. You can also let the script figure out qt settings by itself:

    sh build.sh <qt_dir>

where \<qt_dir\> is the install directory of the Qt library. This simplification applies to other editions too.

Additional flags are needed to build special editions:

#### FlyEM Edition
    
    sh build.sh <qmake_path> <qmake_spec_path> -e flyem

#### FlyEM Edition with DVID support
    
    sh build.sh <qmake_path> <qmake_spec_path> -e flyem -q "DVIDCPP_PATH=<dvidcpp_apth>"
    
Here \<dvidcpp_path\> is the install path of libdvid-cpp.

Manual: https://www.dropbox.com/s/cnewjf7bdm3qbdj/manual.pdf?dl=0

#### Bright Field (Biocytin) Edition

    sh build.sh <qmake_path> <qmake_spec_path> -e biocytin

### Windows

See neurolabi/Compile_Windows.txt for more details.

## Other information
 
The binary version for dark field neuron reconstruction can be downloaded from 

    http://www.neutracing.com

Contact Ting Zhao: zhaot@janelia.hhmi.org for any questions or comments.
