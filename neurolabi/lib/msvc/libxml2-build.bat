cd /d "%~dp0"

set currDIR=%CD%
set srcDIR=%currDIR%\libxml2-2.9.4
set buildDIR=%srcDIR%\..\libxml2-build
set installDIR=%currDIR%\libxml2

rd /q/s %srcDIR%
%currDIR%\7za.exe e -y libxml2-2.9.4.tar.gz && %currDIR%\7za.exe x -y libxml2-2.9.4.tar

rd /q/s %installDIR%

rd /q/s %buildDIR%
md %buildDIR%
cd %buildDIR%

cd %srcDIR%\win32
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
cscript configure.js iconv=no compiler=msvc prefix=%installDIR%
nmake /f Makefile.msvc install

cd %currDIR%
rd /q/s %buildDIR%

echo off
pause
echo The batch file is complete.