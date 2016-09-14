echo %PATH%

cd /d "%~dp0"

set currDIR=%CD%
set srcDIR=%currDIR%\c
set buildDIR=%srcDIR%\..\__neurolabi-build

rd /q/s %buildDIR%
md %buildDIR%
cd %buildDIR%

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cmake -G "Visual Studio 14 2015 Win64" %srcDIR%

MSBuild.exe ALL_BUILD.vcxproj /property:Configuration=Release /property:ForceImportBeforeCppTargets=%currDIR%\runtime_md.props /maxcpucount
MSBuild.exe INSTALL.vcxproj /property:Configuration=Release /property:ForceImportBeforeCppTargets=%currDIR%\runtime_md.props 

cd ..
rd /q/s %buildDIR%

echo off
pause
echo The batch file is complete.
