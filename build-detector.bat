@echo off
setlocal enabledelayedexpansion

set "BUILD_CONFIG=%~1"
if /I "!BUILD_CONFIG!"=="Debug" goto :config_ok
if /I "!BUILD_CONFIG!"=="Release" goto :config_ok
if "!BUILD_CONFIG!"=="" (
    set "BUILD_CONFIG=Release"
) else (
    echo [ERROR] Invalid configuration "%~1".
    echo [*] Usage: build-detector.bat [Debug^|Release]
    exit /b 1
)
:config_ok

set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" (
    echo [ERROR] vswhere.exe was not found.
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo [ERROR] Unable to locate Visual Studio with MSBuild.
    exit /b 1
)

set "MSBUILD_PATH=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"
if not exist "!MSBUILD_PATH!" (
    echo [ERROR] MSBuild was not found at "!MSBUILD_PATH!".
    exit /b 1
)

echo [*] Building detector (!BUILD_CONFIG!^|x64)...
"!MSBUILD_PATH!" "hyper-reV.sln" /t:detector:Rebuild /p:Configuration=!BUILD_CONFIG! /p:Platform=x64 /m /verbosity:minimal
if errorlevel 1 (
    echo [ERROR] detector build failed.
    exit /b 1
)

if not exist "bin\detector.exe" (
    echo [ERROR] Missing artifact: bin\detector.exe
    exit /b 1
)

echo [+++] DETECTOR BUILD SUCCESSFUL [+++]
dir "bin\detector.exe"
