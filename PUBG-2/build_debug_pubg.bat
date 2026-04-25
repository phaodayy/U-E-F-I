@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ========================================
echo   PHAOHACK - PUBG DEBUG BUILD
echo ========================================

set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
if not exist "!MSBUILD_PATH!" (
    set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" set "VSWHERE=C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\amd64\\MSBuild.exe`) do (
            set "MSBUILD_PATH=%%i"
        )
    )
)

taskkill /F /IM GameOverlay_Debug.exe /T >nul 2>&1
timeout /t 1 >nul

echo [*] Rebuilding GameOverlay (Debug)...
"!MSBUILD_PATH!" "phao_final.sln" /t:GameOverlay:Rebuild /p:Configuration=Debug /p:Platform=x64 /m /verbosity:minimal

if errorlevel 1 (
    echo [!] ERROR: Build failed.
    pause
    exit /b 1
)

if exist "%CD%\bin\GameOverlay_Debug.exe" (
    echo [OK] SUCCESS!
    echo [+] Output: bin\GameOverlay_Debug.exe
)
pause
exit /b 0
