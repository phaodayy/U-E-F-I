@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ========================================
echo   UnifiedTelemetryNode - DEBUG BUILD
echo ========================================

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

if not exist "!MSBUILD_PATH!" (
    echo [ERROR] MSBuild.exe not found.
    exit /b 1
)

taskkill /F /IM GameOverlay_Debug.exe /T >nul 2>&1
ping 127.0.0.1 -n 2 >nul

set "STAGING_DIR=%CD%\dist_build\Debug"
set "INT_DIR=%CD%\telemetry\script_build\Debug"
if not exist "!STAGING_DIR!" mkdir "!STAGING_DIR!"

echo [*] Rebuilding GameOverlay (Debug)...
"!MSBUILD_PATH!" "phao_final.sln" /t:GameOverlay:rebuild /p:Configuration=Debug /p:Platform=x64 /p:VcpkgApplocalDeps=false "/p:OutDir=!STAGING_DIR!\\" "/p:IntDir=!INT_DIR!\\" /m /verbosity:minimal

if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

if not exist "!STAGING_DIR!\GameOverlay_Debug.exe" (
    echo [ERROR] Missing build output: !STAGING_DIR!\GameOverlay_Debug.exe
    exit /b 1
)

set "OUTPUT_DIR=%CD%\dist_debug"
if not exist "!OUTPUT_DIR!" mkdir "!OUTPUT_DIR!"
del /F /Q "!OUTPUT_DIR!\*" >nul 2>&1
copy /Y "!STAGING_DIR!\GameOverlay_Debug.exe" "!OUTPUT_DIR!\GameOverlay_Debug.exe" >nul
if errorlevel 1 (
    echo [ERROR] Failed to copy debug output to: !OUTPUT_DIR!\GameOverlay_Debug.exe
    exit /b 1
)
if not exist "!OUTPUT_DIR!\GameOverlay_Debug.exe" (
    echo [ERROR] Missing debug output after copy: !OUTPUT_DIR!\GameOverlay_Debug.exe
    exit /b 1
)

echo [OK] SUCCESS!
echo [+] Single-file debug output: dist_debug\GameOverlay_Debug.exe
exit /b 0
