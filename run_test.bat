@echo off
:: 1. 彻底清空所有可能带来冲突的全局环境变量
set PATH=

:: 2. 只把你的项目编译输出目录、Qt、OCC 的动态库路径塞进去（按你的实际路径对齐）
set PATH=D:\Tools\Qt\6.5.3\msvc2019_64\bin;D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64\win64\vc14\bin;%PATH%

:: 3. 顺便把你的构建目录也加进去（假设你的 exe 在 build-release 或类似目录的 Release 下）
:: 请根据你左侧 CMake 面板生成的实际可执行文件路径进行微调
cd /d "%~dp0"
start build-release\Release\MyCAE.exe