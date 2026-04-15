@echo off
setlocal enabledelayedexpansion

echo ========================================
echo      hyper-reV All-in-One Builder
echo ========================================

:: 1. Find MSBuild Path
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

set "MSBUILD_PATH=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"

echo [*] Found MSBuild: !MSBUILD_PATH!

:: 2. Build EDK-II Libraries
echo [*] Step 1: Building EDK-II Libraries...
"!MSBUILD_PATH!" uefi-boot\ext\edk2\build\EDK-II.sln /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal

:: 3. Build hyper-reV Solution
echo [*] Step 2: Building hyper-reV Project...
"!MSBUILD_PATH!" hyper-reV.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal

echo.
echo [*] Step 3: Finalizing bin folder...
if not exist bin mkdir bin

:: Copy outputs from their respective build folders
copy /Y uefi-boot\build\x64\Release\uefi-boot.efi bin\ >nul 2>&1
copy /Y hyperv-attachment\build\x64\Release\hyperv-attachment.dll bin\ >nul 2>&1
copy /Y x64\Release\usermode.exe bin\ >nul 2>&1
copy /Y bin\usermode.exe bin\ >nul 2>&1
copy /Y x64\Release\loader.exe bin\ >nul 2>&1

echo.
echo [+++] ALL PROJECTS BUILT SUCCESSFULLY! [+++]
dir bin\*.exe bin\*.dll bin\*.efi
echo.
pause
