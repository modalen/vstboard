set NSIS_PATH="c:\Program Files (x86)\NSIS\makensis.exe"
set BUILD_PATH=..\vstboard.32-2010\buildall-build-desktop
set JOM_PATH="E:\Qt\qtcreator-2.2.1\bin\jom"

call "e:\Qt\4.7.3-32-2010\bin\qtvars.bat"
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /xp /x86 /Release

mkdir "%BUILD_PATH%/installer"
copy /y "*.txt" "%BUILD_PATH%/installer"
copy /y ".\tools\nsis.nsi" "%BUILD_PATH%/installer"

pushd "libs\qtwinmigrate\buildlib"
qmake -r buildlib.pro
%JOM_PATH% clean
%JOM_PATH% release
popd

cd "%BUILD_PATH%"
qmake -r C:\Users\CtrlBrk\Documents\VstBoard\src\buildall.pro
%JOM_PATH% clean
%JOM_PATH% release

pause

copy /y ".\dllLoader\release\VstBoardEffect.dll" ".\installer"
copy /y ".\dllLoaderInst\release\VstBoardInstrument.dll" ".\installer"
copy /y ".\vstboard\release\vstboard.exe" ".\installer"
copy /y ".\vstdll\release\VstBoardPlugin.dll" ".\installer"
copy /y "%QTDIR%\bin\QtCore4.dll" ".\installer"
copy /y "%QTDIR%\bin\QtGui4.dll" ".\installer"
copy /y "%QTDIR%\bin\QtScript4.dll" ".\installer"
copy /y "%QTDIR%\bin\QtSolutions_MFCMigrationFramework-head.dll" ".\installer"
rem copy /y "%WindowsSDKDir%\Redist\VC\vcredist_x86.exe" ".\installer"

cd "installer"
%NSIS_PATH% /DARCH=x86 /DVERSION="0.4.9" nsis.nsi

pause
