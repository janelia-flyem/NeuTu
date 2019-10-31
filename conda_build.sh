#!/bin/bash
set -e
if [ $# == 0 ]
then
  echo "Usage: sh conda_build.sh <neutu/neu3/neutu-develop/neu3-develop/neutu-debug/neu3-debug>"
  exit 1
fi

export NEUTU_TARGET=$1

recipe=recipe-neutu
conda build $recipe -c flyem-forge -c conda-forge
 
