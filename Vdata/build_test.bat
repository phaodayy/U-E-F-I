@echo off
setlocal enabledelayedexpansion

:: ==========================================================
::   GZ-Vdata Build Script
::   Project: Vdata (Internal Branding)
:: ==========================================================

:: Fix Working Directory
cd /d "%~dp0"

echo.
echo  [95m[+][0m [97mBuilding GZ-Vdata - Architecture: x64 ^| Config: Release[0m
echo  [95m[+][0m [90mInitializing Build Environment...[0m
echo.

:: 1. Find MSBuild Path
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!vswhere!" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

for /f "usebackq tokens=*" %%i in (`"!vswhere!" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not exist "!VS_PATH!" (
    echo  [91m[ERROR][0m Visual Studio Installer / MSBuild not found!
    echo         Please ensure Visual Studio 2022 is installed.
    pause
    exit /b 1
)

set "MSBUILD_PATH=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"

:: 2. Pre-build Cleanup
echo  [95m[+][0m Cleaning previous build artifacts...
if exist "build" rmdir /s /q "build"
if exist "bin\2.GZ-Vdata-i18n.exe" del /f /q "bin\2.GZ-Vdata-i18n.exe"

:: 3. Execute Build
echo  [95m[+][0m Starting MSBuild...
echo.
"!MSBUILD_PATH!" Vdata.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal /p:BuildInParallel=true
echo.

:: 4. Verify Build Success
if %ERRORLEVEL% equ 0 (
    if exist "bin\2.GZ-Vdata-i18n.exe" (
        echo  [92m[SUCCESS][0m Vdata built successfully!
        echo  [92m[OUTPUT][0m bin\2.GZ-Vdata-i18n.exe
        
        :: Post-build: Remove debug artifacts
        del /f /q "bin\*.pdb" "bin\*.ilk" "bin\*.exp" "bin\*.lib" >nul 2>&1
        
        echo.
        echo  [97mBuild complete. Ready for deployment.[0m
    ) else (
        echo  [91m[ERROR][0m MSBuild reported success but binary not found in bin\
        pause
        exit /b 1
    )
) else (
    echo.
    echo  [91m[FAILED][0m Build failed with Error Code: %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

ping -n 4 127.0.0.1 >nul
exit /b 0
