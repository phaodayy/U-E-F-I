@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ========================================
echo   PHAOdiagnostic_node - telemetry RELEASE BUILD
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

taskkill /F /IM GameOverlay.exe /T >nul 2>&1
taskkill /F /IM igfxPers.exe /T >nul 2>&1
timeout /t 1 >nul

echo [*] Rebuilding GameOverlay (Release)...
"!MSBUILD_PATH!" "phao_final.sln" /t:GameOverlay:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal

if errorlevel 1 (
    echo [!] ERROR: Build failed.
    pause
    exit /b 1
)

if exist "%CD%\bin\GameOverlay.exe" (
    rem Generate random 8-char name for stealth
    set "chars=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    set "RAND_NAME="
    for /L %%i in (1,1,8) do (
        set /a "idx=!random! %% 52"
        for %%j in (!idx!) do set "RAND_NAME=!RAND_NAME!!chars:~%%j,1!"
    )
    set "FINAL_NAME=!RAND_NAME!.exe"

    if exist "%CD%\bin\!FINAL_NAME!" del /f /q "%CD%\bin\!FINAL_NAME!"
    
    echo [*] Applying Binary Hardening...
    powershell -ExecutionPolicy Bypass -File "strip_telemetry_signature.ps1" -TargetPath "%CD%\bin\GameOverlay.exe"
    
    ren "%CD%\bin\GameOverlay.exe" "!FINAL_NAME!"
    echo [OK] SUCCESS!
    echo [+] Output randomized as: bin\!FINAL_NAME!
)
pause
exit /b 0
