#!/bin/bash
set -e
if [ $# == 0 ]
then
  echo "Usage: sh conda_build.sh <neutu/neu3/neutu-develop/neu3-develop/neutu-debug/neu3-debug/neutube-develop>"
  exit 1
fi

export NEUTU_TARGET=$1

recipe=recipe-neutu
if [ `uname` = Darwin ]; then
  if [ -z "$THREAD_COUNT" ]
  then
    export THREAD_COUNT=1
  fi
fi
conda build $recipe -c flyem-forge -c conda-forge
 
