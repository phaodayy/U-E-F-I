@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   PUBG-2 PROJECT BUILD SYSTEM
echo ===================================================
echo.

:: Check for vswhere.exe
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo [ERROR] vswhere.exe not found. Visual Studio Installer is missing?
    goto :FAIL
)

echo [1/3] Locating MSBuild...
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILD_PATH="%%i"
)

if not defined MSBUILD_PATH (
    echo [ERROR] MSBuild was not found. Please check your Visual Studio installation.
    goto :FAIL
)

echo [+] Found: %MSBUILD_PATH%
echo.

echo [2/3] Cleaning and Building Solution (Release x64)...
%MSBUILD_PATH% phao_final.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /v:minimal

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Compilation failed with Error Code: %ERRORLEVEL%
    goto :FAIL
)

echo.
echo [3/3] Build Successful!
echo [+] Output folder: d:\HyperVesion\UEFI\hyper-reV\PUBG-2\bin\
echo ===================================================
echo Task completed successfully.
pause
exit /b 0

:FAIL
echo ===================================================
echo Build FAILED. Check the errors above.
echo ===================================================
pause
exit /b 1
