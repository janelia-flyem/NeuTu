#!/bin/bash
function initdir
{
  dirname=$1
  echo "Checking $dirname"
  x=`mkdir -p $dirname 2>&1 || true`
  username=$USER
  if [[ "$x" == *"Permission denied"* ]]
  then
    sudo ./sudo_initdir $dirname $username
  fi
}

set -e 


if [ `uname` = 'Darwin' ]; then
  scriptDir=$(cd $(dirname "$0") && pwd -P)
  export PATH=/usr/bin:/usr/local/bin:/bin:/sbin
else
  scriptDir=$(dirname `readlink -f "$0"`)
  export PATH=/usr/bin:/usr/local/bin:/bin
fi

if test $# -eq 0
then
  install_dir=/opt
else
  install_dir=$1
fi

if [ "${install_dir:0:1}" = "/" ]
then
  install_dir=$install_dir
else
  install_dir=$PWD/$install_dir
fi

if [ $# -gt 1 ]
then
  package=$2
fi

if [ $# -gt 2 ]
then
  channel=$3
fi

echo "Installing NeuTu under $install_dir"
initdir $install_dir
bindir=$install_dir/bin
initdir $bindir


downloadDir=$install_dir/Download
initdir $downloadDir

if [ -z $condaDir ]
then
  condaDir=$install_dir/Download/miniconda
fi
#echo $PATH
#which md5
#exit
ARCH=`uname -m`
cd $downloadDir
if [ ! -d $condaDir ]
then
  if [ `uname` = 'Darwin' ]; then
    curl -X GET https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-${ARCH}.sh > Miniconda-latest-${ARCH}.sh
  else
    curl -X GET https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-${ARCH}.sh > Miniconda-latest-${ARCH}.sh
  fi
  bash Miniconda-latest-${ARCH}.sh -b -p $condaDir
fi

cd $scriptDir
condarc=$condaDir/.condarc
echo 'channels:' > $condarc
echo '  - flyem-forge' >> $condarc
echo '  - conda-forge' >> $condarc
echo '  - defaults' >> $condarc
#cp condarc $condarc

channel_arg=''
if [ ! -z $channel ]
then
  channel_arg="-c $channel"
fi

if [ -z $package ]
then
  package=neutu
fi

envName='neutu-env'
source $condaDir/bin/activate root
set +e
conda activate $envName
if [[ $? -eq 0 ]]
then
  set -e
  conda install $package -y
else
  set -e
  conda create -n $envName $package -y
fi

appName=neutu
updateFile=$bindir/ntupd
formalName=NeuTu
if [[ $package == neu3* ]]
then
  updateFile=$bindir/n3upd
  appName=neu3
  formalName=Neu3
fi

touch $updateFile
echo '#!/bin/bash' > $updateFile
echo "source  $condaDir/bin/activate $envName" >> $updateFile
echo "conda update $package -y $channel_arg" >> $updateFile
chmod u+x $updateFile

if [ `uname` = 'Darwin' ]
then
  neutu_bin_dir=$condaDir"/envs/neutu-env/bin/${appName}.app/Contents/MacOS"
else
  neutu_bin_dir=$condaDir'/envs/neutu-env/bin'
fi

run_script=$bindir/$appName
touch $run_script
echo '#!/bin/bash' > $run_script
echo ''
echo 'export QT_BEARER_POLL_TIMEOUT=999999999' >> $run_script
if curl -X HEAD -I http://config.int.janelia.org/config/workday | grep '200 OK'; then
  echo 'export NEUPRINT=https://emdata1.int.janelia.org:11000' >> $run_script
  echo 'NEUTU_USER_INFO_ENTRY=http://config.int.janelia.org/config/workday' >> $run_script
fi
echo "source $condaDir/bin/activate $envName" >> $run_script
echo ${neutu_bin_dir}/${appName}' "$@"' >> $run_script
chmod a+x $run_script
echo "Congratuations! Now you can launch $formalName by running '$run_script'"
