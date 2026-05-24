
git submodule update --init --recursive
 
git submodule foreach --recursive "git pull origin master"  

git submodule foreach --recursive "git checkout master"  




echo off & color 0A
::变量延迟的启动语句
setlocal enabledelayedexpansion
::获取.git路径
set gitPath=%cd%\.git
::获取.hooks路径
set hooksPath=%cd%\.hooks\
::判断.hooks文件是否存在
if exist %hooksPath% (
    echo %hooksPath%
) else (
    exit 0
)
::判断.git文件是否存在
if exist %gitPath%\ (
    echo %gitPath%\
) else if exist %gitPath% (
    for /F "tokens=2" %%i in ('type ".\.git"') do (
        set gitPath=%%i
    )
) else (
    exit 0
)
::递归从gitPath查找hooks文件夹，将hooksPath文件夹里的文件copy到hooks文件中
for /R %gitPath% %%f in (hooks) do (
    if exist %%f (
        echo %%f
        copy %hooksPath% %%f
    )
)

endlocal
pause
