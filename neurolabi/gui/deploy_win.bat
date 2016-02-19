cd /d "%~dp0"
set currDIR=%CD%

set qmakeCmd=%1
set buildDIR=%2
set "qmakeCmd=%qmakeCmd:/=\%"
set "buildDIR=%buildDIR:/=\%"
set deployDIR=%currDIR%\..\..\neuTube_win64
set libDIR=%currDIR%\..\lib\Mingw\64\bin
for /f %%i in ('%qmakeCmd% -query QT_INSTALL_BINS') do set qtbinDIR=%%i
for /f %%i in ('%qmakeCmd% -query QT_INSTALL_PLUGINS') do set qtpluginsDIR=%%i
set "qtbinDIR=%qtbinDIR:/=\%"
set "qtpluginsDIR=%qtpluginsDIR:/=\%"

rd /q/s %deployDIR%
md %deployDIR%
md %deployDIR%\plugins
md %deployDIR%\doc

xcopy %qtpluginsDIR% %deployDIR%\plugins /i /e
xcopy %buildDIR%\release\neuTube.exe %deployDIR%
xcopy %currDIR%\config.xml %deployDIR%
xcopy %currDIR%\doc\ThirdPartyLibraries.txt %deployDIR%\doc
xcopy %qtbinDIR%\libgcc_s_seh-1.dll %deployDIR%
xcopy %qtbinDIR%\libstdc++-6.dll %deployDIR%
xcopy %qtbinDIR%\libwinpthread-1.dll %deployDIR%
xcopy %qtbinDIR%\QtCore4.dll %deployDIR%
xcopy %qtbinDIR%\QtGui4.dll %deployDIR%
xcopy %qtbinDIR%\QtNetwork4.dll %deployDIR%
xcopy %qtbinDIR%\QtOpenGL4.dll %deployDIR%
xcopy %qtbinDIR%\QtXml4.dll %deployDIR%
xcopy %libDIR%\libfftw3-3.dll %deployDIR%
xcopy %libDIR%\libfftw3f-3.dll %deployDIR%
xcopy %libDIR%\libxml2.dll %deployDIR%
xcopy %libDIR%\libzlib.dll %deployDIR%

cd /d %deployDIR%\plugins
del /F /S /Q *d4.dll