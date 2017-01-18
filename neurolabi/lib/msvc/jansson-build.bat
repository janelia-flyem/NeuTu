cd /d "%~dp0"

set currDIR=%CD%
set srcDIR=%currDIR%\jansson-2.8
set buildDIR=%srcDIR%\..\jansson-build
set installDIR=%currDIR%\jansson

rd /q/s %srcDIR%
%currDIR%\7za.exe x -y jansson-2.8.zip

rd /q/s %installDIR%

rd /q/s %buildDIR%
md %buildDIR%
cd %buildDIR%

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX=%installDIR% %srcDIR%

MSBuild.exe ALL_BUILD.vcxproj /property:Configuration=Release /maxcpucount
MSBuild.exe INSTALL.vcxproj /property:Configuration=Release

cd %currDIR%
rd /q/s %buildDIR%

echo off
pause
echo The batch file is complete.