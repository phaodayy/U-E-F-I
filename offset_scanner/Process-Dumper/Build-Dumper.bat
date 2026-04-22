@echo off
setlocal enabledelayedexpansion

echo ================================================
echo      Process-Dumper (Hyper-reV Version) Build
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

:: 2. Build ProcessDumper Project
echo [*] Cleaning and Building ProcessDumper...
"!MSBUILD_PATH!" ProcessDumper.sln /t:ProcessDumper:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal

if %ERRORLEVEL% neq 0 (
    echo.
    echo [!] BUILD FAILED! Please check the errors above.
    echo.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [+++] SUCCESS: ProcessDumper built successfully! [+++]
echo [*] Output: x64\Release\ProcessDumper.exe

if not exist "bin" mkdir "bin"
copy /Y x64\Release\ProcessDumper.exe bin\ >nul 2>&1

echo.
dir bin\ProcessDumper.exe
echo.
:: pause
