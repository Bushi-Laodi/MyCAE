@echo off
title MyCAE - No jemalloc Test
setlocal enabledelayedexpansion

set "TARGET_DIR=D:\project\OCCplusVTK\MyCAE\build-release\Release"
set "QT_DIR=D:\Tools\Qt\6.5.3\msvc2019_64"
set "VTK_DIR=D:\Tools\VTK\vtk-9.5.2-qt-release"
set "OCC_DIR=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64"
set "OCC_3RD=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\3rdparty-vc14-64"

:: ============================================
:: Step 1: Remove jemalloc.dll (most likely cause of 0xC0000142)
:: ============================================
echo [CLEANUP] Removing jemalloc.dll (memory allocator - conflicts with CRT)...

if exist "%TARGET_DIR%\jemalloc.dll" (
    echo   Removing jemalloc.dll
    del "%TARGET_DIR%\jemalloc.dll"
)

:: Also remove ANGLE and debug DLLs
if exist "%TARGET_DIR%\libegl.dll" del "%TARGET_DIR%\libegl.dll"
if exist "%TARGET_DIR%\libglesv2.dll" del "%TARGET_DIR%\libglesv2.dll"
if exist "%TARGET_DIR%\tbb12_debug.dll" del "%TARGET_DIR%\tbb12_debug.dll"
if exist "%TARGET_DIR%\tbbmalloc_debug.dll" del "%TARGET_DIR%\tbbmalloc_debug.dll"
if exist "%TARGET_DIR%\tbbmalloc_proxy_debug.dll" del "%TARGET_DIR%\tbbmalloc_proxy_debug.dll"
if exist "%TARGET_DIR%\TKOpenGl.dll" del "%TARGET_DIR%\TKOpenGl.dll"
if exist "%TARGET_DIR%\TKOpenGles.dll" del "%TARGET_DIR%\TKOpenGles.dll"

echo [CLEANUP] Done.

:: ============================================
:: Step 2: Build minimal PATH
:: ============================================
set "PATH=C:\Windows\System32"
set "PATH=%PATH%;C:\Windows"
set "PATH=%PATH%;%TARGET_DIR%"
set "PATH=%PATH%;%QT_DIR%\bin"
set "PATH=%PATH%;%VTK_DIR%\bin"
set "PATH=%PATH%;%OCC_DIR%\win64\vc14\bin"
set "PATH=%PATH%;%OCC_3RD%\tbb-2021.13.0-x64\bin"
set "PATH=%PATH%;%OCC_3RD%\tcltk-8.6.15-x64\bin"
set "PATH=%PATH%;%OCC_3RD%\freetype-2.13.3-x64\bin"
set "PATH=%PATH%;%OCC_3RD%\freeimage-3.18.0-x64\bin"
set "PATH=%PATH%;%OCC_3RD%\ffmpeg-3.3.4-64\bin"

set "QT_PLUGIN_PATH=%TARGET_DIR%"
set "QT_QPA_PLATFORM_PLUGIN_PATH=%TARGET_DIR%\platforms"

echo.
echo Starting MyCAE.exe...
echo.

cd /d "%TARGET_DIR%"
MyCAE.exe
echo.
echo Exit code: %ERRORLEVEL%
pause
