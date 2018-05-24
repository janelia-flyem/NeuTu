cd /d "%~dp0"

set currDIR=%CD%
set srcDIR=%currDIR%\fftw-3.3.5-dll64
set buildDIR=%srcDIR%\..\fftw-build
set installDIR=%currDIR%\fftw

rd /q/s %installDIR%
%currDIR%\7za.exe x -y -o%installDIR% fftw-3.3.5-dll64.zip

cd %installDIR%

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

lib /machine:x64 /def:libfftw3-3.def
lib /machine:x64 /def:libfftw3f-3.def
lib /machine:x64 /def:libfftw3l-3.def

cd %currDIR%
rd /q/s %buildDIR%

echo off
pause
echo The batch file is complete.