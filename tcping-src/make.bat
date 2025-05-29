@echo off
cls
echo Building tcping for Windows...
echo.

del tcping.exe 2>nul
del version.h 2>nul

echo Copying version.h...
copy ..\tcping-32\version.h . >nul

echo Running nmake...
nmake

echo Cleaning up...
del version.h 2>nul
del *.obj 2>nul

echo.
echo Build completed.
dir tcping.exe

echo.
echo "REMEMBER - Doublecheck that it still works statically in XP if VS2012 or above."
