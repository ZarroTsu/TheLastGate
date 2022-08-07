@echo off
SET MINGWBINPATH=C:\MinGW\bin
SET MINGWINCPATH=C:\MinGW\include
SET MINGWLIBPATH=C:\MinGW\lib
SET OUTPATH=TheLastGate

echo ==== Compiling Source with MinGW ====
%MINGWBINPATH%\mingw32-make.exe

echo ==== Cleaning Up obj files ====
del *.o

echo ==== Moving Client ====
move  TheLastGate.exe %OUTPATH%

echo ==================
echo ==== Finished ====
echo ==================
pause
