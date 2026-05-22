@echo off
setlocal enabledelayedexpansion

set TARGET_DIR=D:\project\OCCplusVTK\MyCAE\build-release\Release
set QT_BIN=D:\Tools\Qt\6.5.3\msvc2019_64\bin
set QT_PLUGINS=D:\Tools\Qt\6.5.3\msvc2019_64\plugins
set VTK_BIN=D:\Tools\VTK\vtk-9.5.2-qt-release\bin
set OCC_BIN=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64\win64\vc14\bin
set OCC_3RD=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\3rdparty-vc14-64

echo ========================================
echo Copying DLLs to %TARGET_DIR%
echo ========================================

echo.
echo ===== Step 1: Copying VTK DLLs =====
for %%f in ("%VTK_BIN%\*.dll") do (
    copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)
echo VTK DLLs copied.

echo.
echo ===== Step 2: Copying OCC DLLs =====
for %%f in ("%OCC_BIN%\*.dll") do (
    copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)
echo OCC DLLs copied.

echo.
echo ===== Step 3: Copying OCC 3rd-party DLLs =====

:: ANGLE (OpenGL ES)
if exist "%OCC_3RD%\angle-gles2-2.1.0-vc14-64\bin" (
    echo Copying ANGLE DLLs...
    copy /y "%OCC_3RD%\angle-gles2-2.1.0-vc14-64\bin\libEGL.dll" "%TARGET_DIR%\libegl.dll" >nul 2>&1
    copy /y "%OCC_3RD%\angle-gles2-2.1.0-vc14-64\bin\libGLESv2.dll" "%TARGET_DIR%\libglesv2.dll" >nul 2>&1
    copy /y "%OCC_3RD%\angle-gles2-2.1.0-vc14-64\bin\d3dcompiler_47.dll" "%TARGET_DIR%\" >nul 2>&1
)

:: Tcl/Tk
if exist "%OCC_3RD%\tcltk-8.6.15-x64\bin" (
    echo Copying Tcl/Tk DLLs...
    copy /y "%OCC_3RD%\tcltk-8.6.15-x64\bin\tcl86.dll" "%TARGET_DIR%\" >nul 2>&1
    copy /y "%OCC_3RD%\tcltk-8.6.15-x64\bin\tk86.dll" "%TARGET_DIR%\" >nul 2>&1
)

:: FFmpeg
if exist "%OCC_3RD%\ffmpeg-3.3.4-64\bin" (
    for %%f in ("%OCC_3RD%\ffmpeg-3.3.4-64\bin\*.dll") do copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)

:: FreeImage
if exist "%OCC_3RD%\freeimage-3.18.0-x64\bin" (
    for %%f in ("%OCC_3RD%\freeimage-3.18.0-x64\bin\*.dll") do copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)

:: FreeType
if exist "%OCC_3RD%\freetype-2.13.3-x64\bin" (
    for %%f in ("%OCC_3RD%\freetype-2.13.3-x64\bin\*.dll") do copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)

:: Jemalloc
if exist "%OCC_3RD%\jemalloc-vc14-64\bin" (
    for %%f in ("%OCC_3RD%\jemalloc-vc14-64\bin\*.dll") do copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)

:: OpenVR
if exist "%OCC_3RD%\openvr-1.14.15-64\bin\win64" (
    for %%f in ("%OCC_3RD%\openvr-1.14.15-64\bin\win64\*.dll") do copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)

:: TBB
if exist "%OCC_3RD%\tbb-2021.13.0-x64\bin" (
    for %%f in ("%OCC_3RD%\tbb-2021.13.0-x64\bin\*.dll") do copy /y "%%f" "%TARGET_DIR%\" >nul 2>&1
)

echo OCC 3rd-party DLLs copied.

echo.
echo ===== Step 4: Copying additional Qt DLLs =====
if exist "%QT_BIN%\Qt6Quick.dll" copy /y "%QT_BIN%\Qt6Quick.dll" "%TARGET_DIR%\" >nul 2>&1
if exist "%QT_BIN%\Qt6Sql.dll" copy /y "%QT_BIN%\Qt6Sql.dll" "%TARGET_DIR%\" >nul 2>&1
if exist "%QT_BIN%\Qt6Network.dll" copy /y "%QT_BIN%\Qt6Network.dll" "%TARGET_DIR%\" >nul 2>&1
if exist "%QT_BIN%\Qt6Svg.dll" copy /y "%QT_BIN%\Qt6Svg.dll" "%TARGET_DIR%\" >nul 2>&1
if exist "%QT_BIN%\Qt6OpenGL.dll" copy /y "%QT_BIN%\Qt6OpenGL.dll" "%TARGET_DIR%\" >nul 2>&1
if exist "%QT_BIN%\Qt6OpenGLWidgets.dll" copy /y "%QT_BIN%\Qt6OpenGLWidgets.dll" "%TARGET_DIR%\" >nul 2>&1

echo.
echo ===== Step 5: Copying system DLLs (MSVC runtime) =====
:: NOTE: msvcp140.dll and vcruntime140.dll from C:\Windows\System32 are VS 2015 version (14.0.24215.1)
:: but Qt6 and VTK were compiled with VS 2022 which needs version 14.44.35211.0
:: Copy VS 2022 versions from WinSxS (Microsoft Edge WebView2)
copy /y "C:\Windows\WinSxS\amd64_microsoft-edge-webview_31bf3856ad364e35_10.0.26100.8246_none_2ec91fd13ab0f039\msvcp140.dll" "%TARGET_DIR%\" >nul 2>&1
copy /y "C:\Windows\WinSxS\amd64_microsoft-edge-webview_31bf3856ad364e35_10.0.26100.8246_none_2ec91fd13ab0f039\vcruntime140.dll" "%TARGET_DIR%\" >nul 2>&1
copy /y "C:\Windows\System32\msvcp140_1.dll" "%TARGET_DIR%\" >nul 2>&1
copy /y "C:\Windows\System32\msvcp140_2.dll" "%TARGET_DIR%\" >nul 2>&1
copy /y "C:\Windows\System32\concrt140.dll" "%TARGET_DIR%\" >nul 2>&1
copy /y "C:\Windows\System32\vcruntime140_1.dll" "%TARGET_DIR%\" >nul 2>&1

echo.
echo ===== Step 6: Running windeployqt =====
"%QT_BIN%\windeployqt6.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "%TARGET_DIR%\MyCAE.exe"

echo.
echo ===== Step 7: Cleaning up Debug DLLs =====
echo Removing any Debug DLLs that may have been copied...
if exist "%TARGET_DIR%\msvcp140d.dll" del "%TARGET_DIR%\msvcp140d.dll"
if exist "%TARGET_DIR%\vcruntime140d.dll" del "%TARGET_DIR%\vcruntime140d.dll"
if exist "%TARGET_DIR%\vcruntime140_1d.dll" del "%TARGET_DIR%\vcruntime140_1d.dll"
if exist "%TARGET_DIR%\ucrtbased.dll" del "%TARGET_DIR%\ucrtbased.dll"
if exist "%TARGET_DIR%\qt6cored.dll" del "%TARGET_DIR%\qt6cored.dll"
if exist "%TARGET_DIR%\qt6guid.dll" del "%TARGET_DIR%\qt6guid.dll"
if exist "%TARGET_DIR%\qt6widgetsd.dll" del "%TARGET_DIR%\qt6widgetsd.dll"

:: Clean Debug plugin DLLs from subdirectories
for %%d in (platforms styles imageformats tls networkinformation generic iconengines) do (
    if exist "%TARGET_DIR%\%%d" (
        for %%f in ("%TARGET_DIR%\%%d\*d.dll") do del "%%f" >nul 2>&1
    )
)

echo Debug DLLs cleaned.

echo.
echo ========================================
echo All DLLs copied successfully!
echo ========================================
echo.
echo Now try running: %TARGET_DIR%\MyCAE.exe
pause
