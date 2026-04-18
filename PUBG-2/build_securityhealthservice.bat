@echo off
setlocal EnableExtensions EnableDelayedExpansion

cd /d "%~dp0"

echo ========================================
echo   SecurityHealthService Build Script
echo ========================================

set "TMP_BUILD=%CD%\.tmp_build"
if not exist "%TMP_BUILD%" mkdir "%TMP_BUILD%"
set "TEMP=%TMP_BUILD%"
set "TMP=%TMP_BUILD%"

set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"

if not exist "!MSBUILD_PATH!" (
    set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" set "VSWHERE=C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"

    if exist "!VSWHERE!" (
        for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\amd64\MSBuild.exe`) do (
            set "MSBUILD_PATH=%%i"
        )
    )
)

if not defined MSBUILD_PATH (
    echo [ERROR] Could not find MSBuild.exe
    echo [ERROR] Install Visual Studio Build Tools or Visual Studio 2022.
    exit /b 1
)

echo [*] MSBuild: !MSBUILD_PATH!
echo [*] TEMP: %TEMP%

tasklist /FI "IMAGENAME eq SecurityHealthService.exe" | find /I "SecurityHealthService.exe" >nul
if not errorlevel 1 (
    echo [*] SecurityHealthService.exe is running. Trying to stop it...
    taskkill /F /IM SecurityHealthService.exe >nul 2>&1
    timeout /t 1 /nobreak >nul

    tasklist /FI "IMAGENAME eq SecurityHealthService.exe" | find /I "SecurityHealthService.exe" >nul
    if not errorlevel 1 (
        echo [WARN] SecurityHealthService.exe is still running.
        echo [WARN] Close it manually or run this script as Administrator, then try again.
        exit /b 1
    )
)

echo [*] Building pubg\SecurityHealthService.vcxproj ...
"!MSBUILD_PATH!" "pubg\SecurityHealthService.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

if not exist "%CD%\bin\SecurityHealthService.exe" (
    echo [ERROR] Build completed but output file was not found.
    exit /b 1
)

echo.
echo [+] Build succeeded.
echo [+] Output: %CD%\bin\SecurityHealthService.exe
exit /b 0
