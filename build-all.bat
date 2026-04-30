@echo off
setlocal enabledelayedexpansion

:: Fix Working Directory for Admin Run
cd /d "%~dp0"

echo ========================================
echo      hyper-reV All-in-One Builder
echo ========================================

:: 1. Find MSBuild Path
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not exist "!VS_PATH!" (
    echo [ERROR] Visual Studio not found!
    pause
    exit /b 1
)

set "MSBUILD_PATH=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"
echo [*] Found MSBuild: !MSBUILD_PATH!

:: 2. Build EDK-II Libraries
echo [*] Step 1: Building EDK-II Libraries...
"!MSBUILD_PATH!" "uefi-boot\ext\edk2\build\EDK-II.sln" /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if %ERRORLEVEL% neq 0 (
    echo [ERROR] EDK-II Build Failed!
    pause
    exit /b %ERRORLEVEL%
)

:: 3. Build Components first
echo [*] Step 2: Building Core Components...
"!MSBUILD_PATH!" "uefi-boot\uefi-boot.vcxproj" /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
"!MSBUILD_PATH!" "hyperv-attachment\hyperv-attachment.vcxproj" /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
pushd UnifiedTelemetryNode
call build_mouse_test_auto.bat
popd

if not exist bin mkdir bin
copy /Y "uefi-boot\bin\uefi-boot.efi" bin\ >nul 2>&1
copy /Y "hyperv-attachment\bin\hyperv-attachment.dll" bin\ >nul 2>&1
copy /Y "UnifiedTelemetryNode\bin\mouse_test.exe" bin\ >nul 2>&1

:: 4. Build Launcher/Loader
echo [*] Step 3: Building Loader and Usermode...
"!MSBUILD_PATH!" "loader\loader.vcxproj" /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Loader Build Failed!
    pause
    exit /b %ERRORLEVEL%
)

copy /Y "loader\x64\Release\loader.exe" bin\ >nul 2>&1
del /F /Q "loader\x64\Release\*.pdb" "loader\x64\Release\*.ilk" "loader\x64\Release\*.iobj" "loader\x64\Release\*.ipdb" >nul 2>&1

echo.
echo [+++] ALL PROJECTS BUILT AND PACKAGED SUCCESSFULLY! [+++]
dir "%~dp0bin\*.exe" "%~dp0bin\*.dll" "%~dp0bin\*.efi"
echo.
pause
