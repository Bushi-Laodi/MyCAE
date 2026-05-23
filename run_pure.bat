@echo off
title MyCAE - Pure Environment Launcher (Troubleshooting)
setlocal enabledelayedexpansion

echo ========================================
echo MyCAE - Pure Environment Launcher
echo ========================================
echo.
echo [INFO] This script clears ALL environment variables and only
echo [INFO] keeps essential Windows system paths + library paths.
echo [INFO] Use this to rule out environment variable pollution.
echo.

:: ============================================
:: Step 1: Define paths (hardcoded, no env vars used)
:: ============================================
set "TARGET_DIR=D:\project\OCCplusVTK\MyCAE\build-release\Release"
set "QT_DIR=D:\Tools\Qt\6.5.3\msvc2019_64"
set "VTK_DIR=D:\Tools\VTK\vtk-9.5.2-qt-release"
set "OCC_DIR=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64"
set "OCC_3RD=D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\3rdparty-vc14-64"

:: ============================================
:: Step 2: Verify MyCAE.exe exists
:: ============================================
if not exist "%TARGET_DIR%\MyCAE.exe" (
    echo [ERROR] MyCAE.exe not found at %TARGET_DIR%\MyCAE.exe
    echo [ERROR] Please build the project first.
    pause
    exit /b 1
)

:: ============================================
:: Step 3: Build a minimal, clean PATH
:: ============================================
:: Start with EMPTY path - only Windows core system directories
set "MIN_PATH=C:\Windows\System32"
set "MIN_PATH=%MIN_PATH%;C:\Windows"
set "MIN_PATH=%MIN_PATH%;C:\Windows\System32\Wbem"
set "MIN_PATH=%MIN_PATH%;C:\Windows\System32\WindowsPowerShell\v1.0\"

:: Add MyCAE's own directory (all DLLs are already copied here by CMake)
set "MIN_PATH=%MIN_PATH%;%TARGET_DIR%"

:: Add Qt runtime
set "MIN_PATH=%MIN_PATH%;%QT_DIR%\bin"

:: Add VTK runtime
set "MIN_PATH=%MIN_PATH%;%VTK_DIR%\bin"

:: Add OCC runtime
set "MIN_PATH=%MIN_PATH%;%OCC_DIR%\win64\vc14\bin"

:: Add OCC 3rd-party runtimes
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\ffmpeg-3.3.4-64\bin"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\freeimage-3.18.0-x64\bin"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\freetype-2.13.3-x64\bin"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\jemalloc-vc14-64\bin"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\openvr-1.14.15-64\bin\win64"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\tbb-2021.13.0-x64\bin"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\tcltk-8.6.15-x64\bin"
set "MIN_PATH=%MIN_PATH%;%OCC_3RD%\angle-gles2-2.1.0-vc14-64\bin"

:: ============================================
:: Step 4: Set Qt platform plugin path
:: ============================================
:: windeployqt copies plugins to subdirs under TARGET_DIR
set "QT_PLUGIN_PATH=%TARGET_DIR%"
set "QT_QPA_PLATFORM_PLUGIN_PATH=%TARGET_DIR%\platforms"

:: ============================================
:: Step 5: Launch in a completely clean environment
:: ============================================
echo [INFO] Target: %TARGET_DIR%\MyCAE.exe
echo [INFO] Qt:     %QT_DIR%
echo [INFO] VTK:    %VTK_DIR%
echo [INFO] OCC:    %OCC_DIR%
echo.
echo [INFO] Launching MyCAE.exe with MINIMAL environment...
echo [INFO] Only PATH, QT_PLUGIN_PATH, QT_QPA_PLATFORM_PLUGIN_PATH are set.
echo [INFO] All other env vars (INCLUDE, LIB, LIBPATH, etc.) are CLEARED.
echo.

:: KEY TRICK: Use 'start' with a new cmd.exe that inherits ONLY what we set.
:: The outer script's environment is isolated from the inner process.
echo ========================================
start "" /wait /D"%TARGET_DIR%" cmd.exe /v:on /c ^
    "set PATH=%MIN_PATH% ^
     && set QT_PLUGIN_PATH=%TARGET_DIR% ^
     && set QT_QPA_PLATFORM_PLUGIN_PATH=%TARGET_DIR%\platforms ^
     && set INCLUDE= ^
     && set LIB= ^
     && set LIBPATH= ^
     && set PATHEXT=.COM;.EXE;.BAT;.CMD ^
     && echo [INFO] Inner environment: ^
     && echo [INFO]   PATH=!PATH! ^
     && echo [INFO]   QT_PLUGIN_PATH=!QT_PLUGIN_PATH! ^
     && echo [INFO]   QT_QPA_PLATFORM_PLUGIN_PATH=!QT_QPA_PLATFORM_PLUGIN_PATH! ^
     && echo. ^
     && echo [INFO] Starting MyCAE.exe... ^
     && MyCAE.exe ^
     && echo. ^
     && echo [INFO] MyCAE.exe exited with code: !ERRORLEVEL!"

set "EXIT_CODE=%ERRORLEVEL%"
echo ========================================
echo.
echo [INFO] MyCAE.exe exited with code: %EXIT_CODE%

:: ============================================
:: Step 6: Interpret exit code
:: ============================================
if %EXIT_CODE% NEQ 0 (
    echo [WARNING] Non-zero exit code detected.
    echo.
    echo [TIP] 0xc0000142 = -1073741502 = 3221226505 (STATUS_DLL_INIT_FAILED)
    echo [TIP] 0xc000013a = -1073741510 = 3221225786 (STATUS_STARTUP_FAILURE)
    echo [TIP] 0xc000007b = -1073741701 = 3221225595 (STATUS_BAD_IMAGE_FORMAT - 32/64-bit mismatch)
    echo.
    if %EXIT_CODE% EQU -1073741502 echo [ERROR] 0xc0000142 detected! DLL initialization failure.
    if %EXIT_CODE% EQU 3221226505 echo [ERROR] 0xc0000142 detected! DLL initialization failure.
    if %EXIT_CODE% EQU -1073741510 echo [ERROR] 0xc000013a detected! Application failed to initialize.
    if %EXIT_CODE% EQU 3221225786 echo [ERROR] 0xc000013a detected! Application failed to initialize.
    if %EXIT_CODE% EQU -1073741701 echo [ERROR] 0xc000007b detected! 32/64-bit DLL mismatch.
    if %EXIT_CODE% EQU 3221225595 echo [ERROR] 0xc000007b detected! 32/64-bit DLL mismatch.
) else (
    echo [SUCCESS] MyCAE.exe exited normally.
)

echo.
echo [INFO] If crash persists, try these debugging steps:
echo [INFO]   1. Run: find_crash_dll.py to check for 32-bit DLLs
echo [INFO]   2. Run: copy_dlls.bat to re-copy all DLLs
echo [INFO]   3. Check error.log in the Release directory
echo.
pause
