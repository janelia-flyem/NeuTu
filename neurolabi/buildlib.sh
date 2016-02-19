#!/bin/bash

echo 'Building 3rd-party libraries ...'
cd lib
sh build.sh
cd ..

./update_library 
./update_library --release 

cd cpp/lib
sh build.sh
