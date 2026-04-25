@echo off
setlocal EnableExtensions EnableDelayedExpansion

cd /d "%~dp0"

echo ========================================
echo   GameOverlay DEBUG (Console) Build
========================================

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
    exit /b 1
)

echo [*] Building GameOverlay (Debug/x64) ...
"!MSBUILD_PATH!" "phao_final.sln" /t:GameOverlay:Rebuild /p:Configuration=Debug /p:Platform=x64 /m /verbosity:minimal

if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

if exist "%CD%\bin\GameOverlay_Debug.exe" (
    echo [OK] SUCCESS!
    echo [+] Output: %CD%\bin\GameOverlay_Debug.exe
) else (
    echo [!] ERROR: Output file not found.
)
pause
exit /b 0
