cd /d "%~dp0"
set currDIR=%CD%

set qmakeCmd=%1
set buildDIR=%2
set "qmakeCmd=%qmakeCmd:/=\%"
set "buildDIR=%buildDIR:/=\%"
set deployDIR=%currDIR%\..\..\neuTube_win64
set libDIR=%currDIR%\..\lib\msvc

rd /q/s %deployDIR%
md %deployDIR%
md %deployDIR%\plugins
md %deployDIR%\doc

xcopy %buildDIR%\release\neuTube.exe %deployDIR%

for /f %%i in ('%qmakeCmd% -query QT_INSTALL_BINS') do set qtbinDIR=%%i

%qtbinDIR%\windeployqt %deployDIR%\neuTube.exe

xcopy %libDIR%\fftw\libfftw3-3.dll %deployDIR%
xcopy %libDIR%\fftw\libfftw3f-3.dll %deployDIR%
xcopy %currDIR%\config.xml %deployDIR%
xcopy %currDIR%\doc\ThirdPartyLibraries.txt %deployDIR%\doc
