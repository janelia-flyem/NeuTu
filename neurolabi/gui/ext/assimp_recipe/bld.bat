mkdir build
cd build

set BUILD_CONFIG=Release

cmake .. -G "Ninja" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
    -DCMAKE_INSTALL_PREFIX:PATH="%LIBRARY_PREFIX%" ^
    -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF ^
    -DASSIMP_BUILD_TESTS:BOOL=OFF
if errorlevel 1 exit 1

ninja install
if errorlevel 1 exit 1
