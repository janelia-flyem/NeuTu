NeuTu (a.k.a. neuTube)
=====

[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)

Software package for neuron reconstruction and visualization

## Download

    git clone -b <branch> https://github.com/janelia-flyem/NeuTu.git NeuTu

Here the \<branch\> placeholder should be specified based on which edition you want to build:

### Dark Field Edition (neuTube)

    git clone -b neutube https://github.com/janelia-flyem/NeuTu.git NeuTu

### Bright Field Edition

    git clone -b biocytin https://github.com/janelia-flyem/NeuTu.git NeuTu

### EM Edition

    git clone -b flyem_release https://github.com/janelia-flyem/NeuTu.git NeuTu

    
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

#### EM Edition
    
Please check README in the flyem_release branch: https://github.com/janelia-flyem/NeuTu/tree/flyem_release

#### Bright Field (Biocytin) Edition

    sh build.sh <qmake_path> <qmake_spec_path> -e biocytin

### Windows

See neurolabi/Compile_Windows.txt for more details.

## Other information
 
The binary version for dark field neuron reconstruction can be downloaded from 

    http://www.neutracing.com

Software manual for the FlyEM edition: https://www.dropbox.com/s/cnewjf7bdm3qbdj/manual.pdf?dl=0

Contact Ting Zhao: zhaot@janelia.hhmi.org for any questions or comments.
