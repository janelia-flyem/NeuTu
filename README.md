NeuTu (a.k.a. neuTube)
=====

[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)

Software package for neuron reconstruction and visualization

## Download

    git clone -b public https://github.com/janelia-flyem/NeuTu.git NeuTu

## Build

### Linux and Mac

1. Make sure you have installed Qt 4.8.1+ (Qt 4.8.4 recommended)
2. Go to the NeuTu directory and run

####

    sh build.sh <qmake_path> <qmake_spec_path>
    
by specifying the qmake command path and the corresponding spec path.

### Windows

See neurolabi/Compile_Windows.txt for more details.

## Configuration

### The edition for dark field neuron reconstruction
Copy neurolabi/gui/config.xml to the folder containing the executable 

### The edition for FLyEM
Copy config_flyem.xml and neurolabi/json to the folder containing the executable

## Other information
 
The binary version for dark field neuron reconstruction can be downloaded from 

    http://www.neutracing.com

Contact Ting Zhao: zhaot@janelia.hhmi.org
