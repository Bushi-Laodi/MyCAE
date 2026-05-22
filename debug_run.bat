@echo off
cd /d "D:\project\OCCplusVTK\MyCAE\build-release\Release"
echo ===== Starting MyCAE.exe =====
echo Working directory: %CD%
echo.
echo If you see a popup error, please tell me the error message.
echo.
MyCAE.exe
echo ===== Exit code: %ERRORLEVEL% =====
echo.
echo Checking for error log...
if exist error.log (
    echo ===== Error log contents: =====
    type error.log
) else (
    echo No error.log found
)
echo.
pause
