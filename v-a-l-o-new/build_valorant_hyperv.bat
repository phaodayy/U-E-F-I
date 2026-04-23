@echo off
setlocal enabledelayedexpansion

echo ================================================
echo      Valorant (Hyper-V Version) Build Tool
echo ================================================

:: 1. Find MSBuild Path
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "!vswhere!" (
    echo [!] Error: vswhere.exe not found. Visual Studio Installer required.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

if "!VS_PATH!"=="" (
    echo [!] Error: Visual Studio installation not found.
    pause
    exit /b 1
)

set "MSBUILD_PATH=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"
if not exist "!MSBUILD_PATH!" set "MSBUILD_PATH=!VS_PATH!\MSBuild\15.0\Bin\MSBuild.exe"

echo [*] Found MSBuild: !MSBUILD_PATH!

:: 2. Build Valorant Project
echo [*] Cleaning and Building Valorant Overlay...
echo.

cd /d "%~dp0"
"!MSBUILD_PATH!" GZ-Valorant.sln /t:valorant:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal

if %ERRORLEVEL% neq 0 (
    echo.
    echo [!] BUILD FAILED! Please check the errors above.
    echo.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [+] ========================================
echo [+] SUCCESS: Valorant Overlay built successfully!
echo [+] Output: bin\GZ-Cheat.exe
echo [+] ========================================
echo.
timeout /t 5
