# NeuTu 

Software for proofreading EM connectomics

## Installation

NeuTu can be installed on a Mac or Linux machine.

### The Easiest Way

1. Download <a href="https://raw.githubusercontent.com/janelia-flyem/NeuTu/develop/neurolabi/shell/setup_neutu_dev.sh" download>setup_neutu_dev.sh</a> 

2. Run 'bash setup_neutu_dev.sh <install_dir>', where <install_dir> is the installation directory. 

Once the installation is done, you can launch the program by running

    <install_dir>/bin/neutu

We use conda to manage our package, so you can also install the software in a more manual way with miniconda3.

### Mac (OSX 10.12+ preferred)
    curl -X GET https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh > Miniconda-latest-MacOSX-x86_64.sh
    bash Miniconda-latest-MacOSX-x86_64.sh
    
    #Note: if the following command fails with some import error, you can unset PYTHONHOME and try again.
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
    
    #For future update, you can run 'conda update -n <NAME> -c flyem neutu' after activating miniconda.
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you should be able to lauch the application neutu.app in \<CONDA_ROOT\>/envs/\<NAME\>/bin.

### Linux (Tested on Ubuntu 16.04, Fedora 16+ and Scientific Linux 7)
    curl -X GET https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh > Miniconda-latest-Linux-x86_64.sh
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
 
### Windows

Not supported yet.

## Manual

Quick Start: https://www.dropbox.com/s/cnewjf7bdm3qbdj/manual.pdf?dl=0

Full Manual: https://www.dropbox.com/s/w9sbry69v04tziu/NeuTu%20Manual%202018.pdf?dl=0
