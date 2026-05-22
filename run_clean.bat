@echo off
title MyCAE - Clean Launcher (No ANGLE, No Debug DLLs)
setlocal enabledelayedexpansion

set "TARGET_DIR=D:\project\OCCplusVTK\MyCAE\build-release\Release"
set "QT_DIR=D:\Tools\Qt\6.5.3\msvc2019_64"
set "VTK_DIR=D:\Tools\VTK\vtk-9.5.2-qt-release"
set "OCC_DIR=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64"
set "OCC_3RD=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\3rdparty-vc14-64"

:: ============================================
:: Step 1: Remove problematic DLLs from target dir
:: ============================================
echo [CLEANUP] Removing potentially conflicting DLLs...

:: Remove ANGLE DLLs (conflict with VTK's native OpenGL)
if exist "%TARGET_DIR%\libegl.dll" (
    echo   Removing libegl.dll (ANGLE - conflicts with VTK OpenGL)
    del "%TARGET_DIR%\libegl.dll"
)
if exist "%TARGET_DIR%\libglesv2.dll" (
    echo   Removing libglesv2.dll (ANGLE - conflicts with VTK OpenGL)
    del "%TARGET_DIR%\libglesv2.dll"
)

:: Remove Debug DLLs
if exist "%TARGET_DIR%\tbb12_debug.dll" (
    echo   Removing tbb12_debug.dll
    del "%TARGET_DIR%\tbb12_debug.dll"
)
if exist "%TARGET_DIR%\tbbmalloc_debug.dll" (
    echo   Removing tbbmalloc_debug.dll
    del "%TARGET_DIR%\tbbmalloc_debug.dll"
)
if exist "%TARGET_DIR%\tbbmalloc_proxy_debug.dll" (
    echo   Removing tbbmalloc_proxy_debug.dll
    del "%TARGET_DIR%\tbbmalloc_proxy_debug.dll"
)

:: Remove OCC OpenGL DLLs (may conflict with VTK)
if exist "%TARGET_DIR%\TKOpenGl.dll" (
    echo   Removing TKOpenGl.dll (OCC OpenGL - may conflict with VTK)
    del "%TARGET_DIR%\TKOpenGl.dll"
)
if exist "%TARGET_DIR%\TKOpenGles.dll" (
    echo   Removing TKOpenGles.dll
    del "%TARGET_DIR%\TKOpenGles.dll"
)

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

:: Intentionally NOT adding ANGLE, jemalloc, openvr paths

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
