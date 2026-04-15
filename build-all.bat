@echo off
setlocal enabledelayedexpansion

echo ========================================
echo      hyper-reV All-in-One Builder
echo ========================================

:: 1. Find MSBuild Path using vswhere
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" (
    echo [!] Error: vswhere.exe not found. Please install Visual Studio Installer.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

set "MSBUILD_PATH=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"

if not exist "!MSBUILD_PATH!" (
    echo [!] Error: MSBuild.exe not found at !MSBUILD_PATH!
    pause
    exit /b 1
)

echo [*] Found MSBuild: !MSBUILD_PATH!

:: 2. Build EDK-II Libraries
echo [*] Step 1: Building EDK-II Libraries...
"!MSBUILD_PATH!" uefi-boot\ext\edk2\build\EDK-II.sln /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if %errorlevel% neq 0 (
    echo [!] Error: Failed to build EDK-II libraries.
    pause
    exit /b 1
)

:: 3. Build hyper-reV Solution
echo [*] Step 2: Building hyper-reV Project...
"!MSBUILD_PATH!" hyper-reV.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if %errorlevel% neq 0 (
    echo [!] Error: Failed to build hyper-reV solution.
    pause
    exit /b 1
)

echo.
echo [*] Step 3: Finalizing bin folder...
if not exist bin mkdir bin
copy /Y uefi-boot\bin\uefi-boot.efi bin\
copy /Y hyperv-attachment\bin\hyperv-attachment.dll bin\
copy /Y usermode\bin\usermode.exe bin\

echo.
echo [+++] ALL PROJECTS BUILT SUCCESSFULLY! [+++]
echo Files are located in the 'bin' folder.
echo.
pause
