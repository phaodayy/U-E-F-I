@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ========================================
echo   Mouse Injection Test - Quick Build
echo ========================================

:: Tìm MSVC compiler
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not exist "!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat" (
    echo [ERROR] Visual Studio C++ build tools not found.
    pause
    exit /b 1
)

:: Thiết lập môi trường 64-bit
call "!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat"

:: Biên dịch
echo [*] Compiling Mouse Test...
ml64.exe /c /Fo hypercall_entry.obj "telemetry\sdk\memory\hypercall_entry.asm"
cl.exe /EHsc /Ox /W3 /I. /Itelemetry /I.. /I..\shared "mouse_test.cpp" "telemetry\sdk\memory\hypercall_bridge.cpp" hypercall_entry.obj /Fe:mouse_test.exe /link /SUBSYSTEM:CONSOLE User32.lib

if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo [SUCCESS] mouse_test.exe has been created.
del *.obj
echo.
echo [!] To test: Run mouse_test.exe (Make sure Hypervisor is loaded)
pause
