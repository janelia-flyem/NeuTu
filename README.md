NeuTu (a.k.a. neuTube)
=====

[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)

Software package for neuron reconstruction and visualization

## Download

    git clone -b biocytin https://github.com/janelia-flyem/NeuTu.git NeuTu

## Build

### Linux and Mac

1. Make sure you have installed Qt 4.8.1+ (Qt 4.8.4 recommended)

    Various versions of Qt can be dowloaded from https://download.qt.io/archive/qt/
    
2. Go to the NeuTu directory and run

####

    sh build.sh <qmake_path> <qmake_spec_path> -e biocytin
by specifying the qmake command path and the corresponding spec path, or

    sh build.sh <qt_dir> -e biocytin
    
### Windows (NOT tested)

See neurolabi/Compile_Windows.txt for more details.

## Other information
 
The binary version for dark field neuron reconstruction can be downloaded from 

    http://www.neutracing.com

Contact Ting Zhao: zhaot@janelia.hhmi.org for any questions or comments.
