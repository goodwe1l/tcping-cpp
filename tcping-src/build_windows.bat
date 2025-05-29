@echo off
REM Windows build script for tcping
REM This script builds tcping on Windows using MSVC

echo Starting Windows build for tcping...

REM Set up Visual Studio environment
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Visual Studio not found, trying with default compiler...
)

REM Clean old files
del /Q *.obj *.exe 2>nul

REM Compile
echo Compiling...
cl /EHsc /O2 /std:c++11 ^
   /D "OS_WINDOWS" ^
   /D "_CRT_SECURE_NO_WARNINGS" ^
   /I. ^
   main.cpp tcping.cpp base64.cpp tee.cpp ws-util.cpp ^
   /link ws2_32.lib ^
   /out:tcping.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Binary: tcping.exe
    dir tcping.exe
) else (
    echo Build failed!
    exit /b 1
)
