@echo off
setlocal
set "VC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VC_PATH%" set "VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

call "%VC_PATH%"

echo [*] Cleaning and Organizing...
if not exist "bin" mkdir "bin"
if not exist "build" mkdir "build"

echo [*] Building Offset Scanner (Hyper-reV Version)...

:: Compile ASM first
ml64 /c /nologo /Fo build\hypercall_entry.obj "D:\HyperVesion\UEFI\U-E-F-I\PUBG-2\pubg\sdk\hypercall_entry.asm"

:: Compile CPP files
cl /EHsc /O2 /MT /std:c++17 ^
    /I "D:\HyperVesion\UEFI\U-E-F-I\PUBG-2" ^
    /I "D:\HyperVesion\UEFI\U-E-F-I\PUBG-2\pubg" ^
    /I "D:\HyperVesion\UEFI\U-E-F-I\PUBG-2\pubg\sdk" ^
    /I "D:\HyperVesion\UEFI\U-E-F-I\shared" ^
    /Fo"build\\" ^
    main.cpp ^
    "D:\HyperVesion\UEFI\U-E-F-I\PUBG-2\pubg\sdk\hypercall_bridge.cpp" ^
    "D:\HyperVesion\UEFI\U-E-F-I\PUBG-2\pubg\sdk\hyper_process.cpp" ^
    build\hypercall_entry.obj ^
    /link /OUT:bin\offset_scanner_smart.exe psapi.lib user32.lib shell32.lib advapi32.lib

if %errorlevel% neq 0 (
    echo [!] Build FAILED!
    exit /b %errorlevel%
)

echo [MD] Build SUCCESS! Output: bin\offset_scanner_smart.exe
echo [*] Cleaning up old root files...
if exist "offset_scanner_smart.exe" del "offset_scanner_smart.exe"
if exist "hyper_process.obj" del "hyper_process.obj"
if exist "main.obj" del "main.obj"
if exist "hypercall_bridge.obj" del "hypercall_bridge.obj"
if exist "hypercall_entry.obj" del "hypercall_entry.obj"

exit /b 0
