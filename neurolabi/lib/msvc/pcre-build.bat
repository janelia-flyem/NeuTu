cd /d "%~dp0"

set currDIR=%CD%
set srcDIR=%currDIR%\pcre2-10.22
set buildDIR=%srcDIR%\..\pcre-build
set installDIR=%currDIR%\pcre

rd /q/s %srcDIR%
7z x -y pcre2-10.22.zip

rd /q/s %installDIR%

rd /q/s %buildDIR%
md %buildDIR%
cd %buildDIR%

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_INSTALL_PREFIX=%installDIR% -DPCRE2_BUILD_PCRE2GREP:BOOL="0" %srcDIR%

MSBuild.exe ALL_BUILD.vcxproj /property:Configuration=Release /maxcpucount
MSBuild.exe INSTALL.vcxproj /property:Configuration=Release

cd %currDIR%
rd /q/s %buildDIR%

echo off
pause
echo The batch file is complete.