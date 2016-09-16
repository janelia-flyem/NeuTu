NeuTu-EM
=====

[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)

Software for proofreading EM connectomics

## Download

    git clone -b flyem_release https://github.com/janelia-flyem/NeuTu.git NeuTu

## Installation

Currently only Mac (OSX 10.10+ preferable) is supported.

### Mac
    wget https://repo.continuum.io/miniconda/Miniconda-latest-MacOSX-x86_64.sh
    bash Miniconda-latest-MacOSX-x86_64.sh
    
    #Assuming miniconda is installed under <CONDA_ROOT>
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you should be able to lauch the application neuTube.app in \<CONDA_ROOT\>/envs/\<NAME\>/bin.

## Build

If the installation does not work for you, you can build the application from scratch.

### Linux

Go to the NeuTu/neurolabi/shell directory and run
 
    ./setup_neutu_j <install_dir>

Here \<install_dir\> must be a clean directory if it already exists. Run \<install_dir\>/bin/neutu to launch the program after installation.

Note: you need to install the following packages using the package manager (e.g. apt-get for Ubuntu and yum/dnf for Fedora) if they have not been installed:
* libxext-dev
* freeglut3-dev
* build-essential for Ubuntu or groupinstall "Development Tools" for Fedora

### Mac

Go to the NeuTu/neurolabi/shell directory and run
 
    ./setup_neutu_conda <install_dir>
    
Here \<install_dir\> must be a clean directory if it already exists. Run \<install_dir\>/bin/neutu to launch the program after installation.
 
### Windows

Not supported yet.

## Manual

https://www.dropbox.com/s/cnewjf7bdm3qbdj/manual.pdf?dl=0

Contact Ting Zhao: zhaot@janelia.hhmi.org for any questions or comments.
