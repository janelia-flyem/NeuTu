NeuTu-EM
=====

<!--[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)-->

Software for proofreading EM connectomics

Publication to cite: [NeuTu: Software for Collaborative, Large-Scale, Segmentation-Based Connectome Reconstruction](https://doi.org/10.3389/fncir.2018.00101), Ting Zhao*, Donald J. Olbris, Yang Yu and Stephen M. Plaza (2018)

## Installation

NeuTu can be installed on a Mac or Linux machine. Its binary releases are available as [conda packages] (https://anaconda.org/flyem-forge/neutu). You can install it manually with conda commands or through the setup script.

### Use Script (The Easiest Way)

1. Download https://raw.githubusercontent.com/janelia-flyem/NeuTu/master/neurolabi/shell/setup_neutu.sh

2. Run 'bash setup_neutu.sh <intall_dir>', where <install_dir> is the installation directory. 

Once the installation is done, you can launch the program by running

    <install_dir>/bin/neutu

### Use Conda Commands

#### Mac (OSX 10.12+ preferred)
    curl -X GET https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh > Miniconda-latest-MacOSX-x86_64.sh
    bash Miniconda-latest-MacOSX-x86_64.sh
    
    #Note: if the following command fails with some import error, you can unset PYTHONHOME and try again.
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
    
    #For future update, you can run 'conda update -n <NAME> -c flyem neutu' after activating miniconda.
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you should be able to lauch the application neutu.app in \<CONDA_ROOT\>/envs/\<NAME\>/bin.

#### Linux (Tested on Ubuntu 16.04, Fedora 16+ and Scientific Linux 7)
    curl -X GET https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh > Miniconda-latest-Linux-x86_64.sh
    bash Miniconda-latest-Linux-x86_64.sh
    
    #Assuming miniconda is installed under <CONDA_ROOT>
    #Note: if the following command fails with some import error, you can unset PYTHONHOME and try again.
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
    
    #For future update, you can run 'conda update -n <NAME> -c flyem neutu' after activating miniconda.
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you can launch the program with the following commands

    source <CONDA_ROOT>/bin/activate <NAME>
    neutu 
 

## Manual

### Quick start
https://github.com/janelia-flyem/NeuTu/blob/master/neurolabi/doc/user_manual/neutu/quick_start.pdf

### Full manual

https://github.com/janelia-flyem/NeuTu/blob/master/neurolabi/doc/user_manual/neutu/manual.pdf

