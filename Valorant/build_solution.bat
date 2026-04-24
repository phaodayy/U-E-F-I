@echo off
setlocal
cd /d "%~dp0"

echo [*] GZ-Valorant Solution Build Script
echo [*] Building all projects (Release x64)...
echo.

:: Setup VS environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 >nul 2>&1

:: Build entire solution
msbuild "GZ-Valorant.sln" /p:Configuration=Release /p:Platform=x64 /m /v:minimal
if errorlevel 1 (
    echo.
    echo [!] Build failed!
    echo Press any key to exit...
    pause >nul
    exit /b 1
)

echo.
echo [+] ========================================
echo [+] Build successful!
echo [+] Output folder: bin\
echo [+]   - GZ-Cheat.exe     (Cheat Overlay)
echo [+] ========================================
timeout /t 2 /nobreak >nul
