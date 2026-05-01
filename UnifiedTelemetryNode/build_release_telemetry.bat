@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ========================================
echo   UnifiedTelemetryNode - USER CONSOLE BUILD
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

taskkill /F /IM GameOverlay.exe /T >nul 2>&1
ping 127.0.0.1 -n 2 >nul

set "STAGING_DIR=%CD%\dist_build\Release"
set "INT_DIR=%CD%\telemetry\script_build\Release"
if not exist "!STAGING_DIR!" mkdir "!STAGING_DIR!"

echo [*] Rebuilding GameOverlay (Release / User Console)...
"!MSBUILD_PATH!" "phao_final.sln" /t:GameOverlay:Rebuild /p:Configuration=Release /p:Platform=x64 /p:VcpkgApplocalDeps=false "/p:OutDir=!STAGING_DIR!\\" "/p:IntDir=!INT_DIR!\\" /m /verbosity:minimal

if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

if not exist "!STAGING_DIR!\GameOverlay.exe" (
    echo [ERROR] Missing build output: !STAGING_DIR!\GameOverlay.exe
    exit /b 1
)

set "OUTPUT_DIR=%CD%\dist"
if not exist "!OUTPUT_DIR!" mkdir "!OUTPUT_DIR!"
del /F /Q "!OUTPUT_DIR!\*" >nul 2>&1
copy /Y "!STAGING_DIR!\GameOverlay.exe" "!OUTPUT_DIR!\GameOverlay.exe" >nul
if errorlevel 1 (
    echo [ERROR] Failed to copy release output to: !OUTPUT_DIR!\GameOverlay.exe
    exit /b 1
)
if not exist "!OUTPUT_DIR!\GameOverlay.exe" (
    echo [ERROR] Missing release output after copy: !OUTPUT_DIR!\GameOverlay.exe
    exit /b 1
)

del /F /Q "!OUTPUT_DIR!\*.pdb" "!OUTPUT_DIR!\*.ilk" "!OUTPUT_DIR!\*.iobj" "!OUTPUT_DIR!\*.ipdb" >nul 2>&1
del /F /Q "!STAGING_DIR!\*.pdb" "!STAGING_DIR!\*.ilk" "!STAGING_DIR!\*.iobj" "!STAGING_DIR!\*.ipdb" >nul 2>&1

echo [OK] SUCCESS!
echo [+] Single-file user console output: dist\GameOverlay.exe
echo [+] Release hardening: CFG/CET, ASLR/NX, no debug artifacts copied.
exit /b 0
