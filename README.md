NeuTu-EM
=====

<!--[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)-->

Software for proofreading EM connectomics

## Download Source Code (Not needed for installation)

    git clone -b flyem_release https://github.com/janelia-flyem/NeuTu.git NeuTu

## Installation

Currently only Mac (OSX 10.10+ preferred) is supported.

### Mac
    curl -X GET https://repo.continuum.io/miniconda/Miniconda2-latest-MacOSX-x86_64.sh > Miniconda-latest-MacOSX-x86_64.sh
    bash Miniconda-latest-MacOSX-x86_64.sh
    
    #Assuming miniconda is installed under <CONDA_ROOT>
    #Note: if the following command fails with some import error, you can unset PYTHONHOME and try again.
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
    
    #For future update, you can run 'conda update -n <NAME> -c flyem neutu' after activating miniconda.
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you should be able to lauch the application neutu.app in \<CONDA_ROOT\>/envs/\<NAME\>/bin.

### Linux
    curl -X GET https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh > Miniconda-latest-Linux-x86_64.sh
    bash Miniconda-latest-Linux-x86_64.sh
    
    #Assuming miniconda is installed under <CONDA_ROOT>
    #Note: if the following command fails with some import error, you can unset PYTHONHOME and try again.
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
    
    #For future update, you can run 'conda update -n <NAME> -c flyem neutu' after activating miniconda.
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you can launch the program with the following commands

    scource <CONDA_ROOT>/bin/activate <NAME>
    neutu 

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
