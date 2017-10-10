NeuTu-EM
=====

<!--[![Build Status](https://drone.io/github.com/janelia-flyem/NeuTu/status.png)](https://drone.io/github.com/janelia-flyem/NeuTu/latest)-->

Software for proofreading EM connectomics

## Installation

NeuTu can be installed on a Mac or Linux machine.

### The Easiest Way

1. Download https://raw.githubusercontent.com/janelia-flyem/NeuTu/develop/neurolabi/shell/setup_neutu_dev

2. Run 'bash setup_neutu_dev <intall_dir>', where <install_dir> is the installation directory. 

Once the installation is done, you can launch the program by running

    <install_dir>/bin/neutu

We use conda to manage our package, so you can also install the software in a more manual way with miniconda3.

### Mac (OSX 10.10+ preferred)
    curl -X GET https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh > Miniconda-latest-MacOSX-x86_64.sh
    bash Miniconda-latest-MacOSX-x86_64.sh
    
    #Note: if the following command fails with some import error, you can unset PYTHONHOME and try again.
    source <CONDA_ROOT>/bin/activate root
    conda create -n <NAME> -c flyem neutu
    
    #For future update, you can run 'conda update -n <NAME> -c flyem neutu' after activating miniconda.
  
Here \<NAME\> is the conda environment name. If you don't know what it is, just use neutu-env.

After successful installation, you should be able to lauch the application neutu.app in \<CONDA_ROOT\>/envs/\<NAME\>/bin.

### Linux (Tested on Ubuntu 16.04 and Fedora 22)
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
 
### Windows

Not supported yet.

## Manual

https://www.dropbox.com/s/cnewjf7bdm3qbdj/manual.pdf?dl=0

Contact Ting Zhao: zhaot@janelia.hhmi.org for any questions or comments.
