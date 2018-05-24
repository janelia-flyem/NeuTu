mkdir build
cd build

set BUILD_CONFIG=Release

cmake .. -G "Ninja" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
    -DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%"
if errorlevel 1 exit 1

ninja install
if errorlevel 1 exit 1
