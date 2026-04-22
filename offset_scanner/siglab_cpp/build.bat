@echo off
setlocal

set "ROOT=%~dp0"
set "VC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VC_PATH%" set "VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VC_PATH%" (
    echo [ERROR] vcvars64.bat not found. Install Visual Studio C++ Build Tools first.
    exit /b 1
)

call "%VC_PATH%"
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC environment.
    exit /b 1
)

if not exist "%ROOT%bin" mkdir "%ROOT%bin"
if not exist "%ROOT%build" mkdir "%ROOT%build"

pushd "%ROOT%"

echo [*] Building siglab (no CMake)...
cl /nologo /EHsc /O2 /MT /std:c++20 ^
    /I "%ROOT%src" ^
    /I "%ROOT%..\..\PUBG-2" ^
    /Fo"build\\" ^
    "src\main.cpp" ^
    "src\pe_image.cpp" ^
    "src\signature.cpp" ^
    /link /OUT:"bin\siglab.exe"

if errorlevel 1 (
    echo [ERROR] Build failed.
    popd
    exit /b 1
)

echo [OK] Build success: "%ROOT%bin\siglab.exe"
echo [INFO] Try: "%ROOT%bin\siglab.exe" --help

popd
endlocal
